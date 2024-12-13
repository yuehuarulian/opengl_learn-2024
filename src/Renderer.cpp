#include "Ray.h"
#include "Scene.h"
#include "Renderer.h"
#include "hittable.h"
#include "Material.h"
#include "Camera.h"
#include <limits>
#include <fstream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <sstream>
#include <iostream>
#include "glm/glm.hpp"

Renderer::Renderer()
	: width(600), height(600), image(nullptr)
{
}

Renderer::~Renderer()
{
	if (image)
		delete image;
}

color Renderer::trace(const Ray &ray, const shared_ptr<Scene> &scene, int depth)
{
	const float reflect_atten = 0.7f;
	const float refract_atten = 0.3f;
	const float ka = 0.05f;
	if (depth >= 5)
	{
		return color(0.0f);
	}

	hit_record rec;
	constexpr float infinity = std::numeric_limits<float>::infinity();

	bool isHit = scene->hit(ray, 0.001f, infinity, rec);
	if (!isHit)
	{
		return background;
	}

	vec3 finalColor = vec3(0.0f);
	// TODO: implement ray tracing and shading algorithm for 3 type of material.
	auto type = rec.mat_ptr->type;
	if (type == MatType::GLASS)
	{
		vec3 normal = normalize(rec.normal);
		float eta = rec.front_face ? rec.mat_ptr->refraction_ratio : 1.0 / rec.mat_ptr->refraction_ratio;

		// 反射光线
		vec3 reflect_dir = glm::reflect(ray.dir, normal);
		Ray reflect_ray(rec.p + reflect_dir * vec3(0.001), reflect_dir);
		vec3 reflect_color = trace(reflect_ray, scene, depth + 1);

		// 折射光线
		vec3 refract_dir = glm::refract(ray.dir, normal, eta);
		Ray refract_ray(rec.p + refract_dir * vec3(0.001), refract_dir);
		vec3 refract_color = trace(refract_ray, scene, depth + 1);

		// 根据反射与折射的比例混合颜色
		finalColor = reflect_atten * reflect_color + refract_atten * refract_color;
	}
	else if (type == MatType::DIFFUSE)
	{
		// 漫反射材质：基于光源的贡献计算颜色
		vec3 normal = normalize(rec.normal);
		vec3 view_dir = -normalize(ray.dir);
		vec3 ambient = ka * background;
		finalColor += ambient;

		for (const auto &light : scene->lights)
		{
			std::shared_ptr<Sphere> light_ptr = std::dynamic_pointer_cast<Sphere>(light);
			vec3 light_dir = normalize(light_ptr->center - rec.p);
			float dis_to_light = std::max(glm::length(light_ptr->center - rec.p) - light_ptr->radius - 0.001f, 0.0f);

			// 漫反射光强度计算：Lambertian 反射模型
			float diffuse_intensity = std::max(dot(normal, light_dir), 0.0f);
			vec3 diffuse_color = diffuse_intensity * vec3(1.0) / (dis_to_light * dis_to_light);

			// 镜面反射部分（Phong/Blinn-Phong）
			vec3 reflect_dir = reflect(-light_dir, normal);
			float spec_intensity = std::max(dot(view_dir, reflect_dir), 0.0f);
			spec_intensity = pow(spec_intensity, 32);
			vec3 specular = spec_intensity * light_ptr->mat_ptr->albedo / (dis_to_light * dis_to_light);

			// Blinn-Phong高光
			vec3 half_dir = normalize(light_dir + view_dir); // 半程向量
			float blinn_spec_intensity = pow(glm::max(glm::dot(normal, half_dir), 0.0f), 32);
			vec3 blinn_specular = blinn_spec_intensity * light_ptr->mat_ptr->albedo / (dis_to_light * dis_to_light);

			// 阴影检测：从交点到光源发射射线，检查是否被遮挡
			Ray shadow_ray(rec.p + 0.001f * (rec.front_face ? 1.0f : -1.0f) * normal, light_dir); // 从物体表面偏移一点，避免自交
			hit_record shadow_rec;
			bool is_shadowed = scene->hit(shadow_ray, 0.001f, dis_to_light, shadow_rec);

			// 如果没有被遮挡，增加光照贡献
			if (!is_shadowed)
			{
				finalColor += (diffuse_color + specular + blinn_specular) * rec.mat_ptr->albedo;
			}
		}
	}
	else if (type == MatType::LIGHT)
	{
		// 光源材质：直接返回颜色
		finalColor = rec.mat_ptr->albedo; // 光源的颜色即为材质的albedo
	}

	// end of TODO
	return finalColor;
}

#include <iostream>
void Renderer::render(const shared_ptr<Scene> &scene)
{
	auto &camera = scene->camera;
	int h = camera->height;
	int w = camera->width;

	image = new int[3 * w * h];

	for (int j = h - 1; j >= 0; j--)
	{
		if (j % 20 == 0)
		{
			std::cout << (h - 1 - j) * 1.0f / (h - 1) << "%\n";
		}
		for (int i = 0; i < w; i++)
		{
			vec3 color(0, 0, 0);
			auto u = float(i) / (w - 1);
			auto v = float(j) / (h - 1);
			Ray r = camera->get_ray(u, v);
			// std::cout << r.dir.x << ' ' << r.dir.y << ' ' << r.dir.z << '\n';
			color = trace(r, scene, 0);

			int pos = j * w + i;
			// write_color(color, pos);
			color = glm::clamp(glm::sqrt(color), 0.0f, 1.0f);
			image[3 * pos] = 255 * color.x;
			image[3 * pos + 1] = 255 * color.y;
			image[3 * pos + 2] = 255 * color.z;
		}
	}
}

void Renderer::writePic(const string &filename, const shared_ptr<Scene> &scene)
{
	std::cout << "Writing to file: " << filename << '\n';
	auto &camera = scene->camera;

	std::ofstream f(filename);
	int h = camera->height;
	int w = camera->width;
	f << "P3\n"
	  << camera->width << ' ' << camera->height << "\n255\n";

	for (int j = h - 1; j >= 0; j--)
	{
		for (int i = 0; i < w; i++)
		{
			int index = j * w + i;
			f << image[3 * index] << ' ' << image[3 * index + 1]
			  << ' ' << image[3 * index + 2] << '\n';
		}
	}

	std::vector<unsigned char> pixels(w * h * 3);
	// Copy data from output_frame_ptr to pixels
	for (int i = 0; i < w * h; ++i)
	{
		pixels[i * 3 + 0] = static_cast<unsigned char>(glm::clamp(image[3 * i] * 1.0f, 0.0f, 255.0f));	   // Red
		pixels[i * 3 + 1] = static_cast<unsigned char>(glm::clamp(image[3 * i + 1] * 1.0f, 0.0f, 255.0f)); // Green
		pixels[i * 3 + 2] = static_cast<unsigned char>(glm::clamp(image[3 * i + 2] * 1.0f, 0.0f, 255.0f)); // Blue
	}
	// Save the image to disk
	std::ostringstream oss;
	oss << "image.png";
	stbi_flip_vertically_on_write(true);
	stbi_write_png(oss.str().c_str(), w, h, 3, pixels.data(), w * 3);
}
