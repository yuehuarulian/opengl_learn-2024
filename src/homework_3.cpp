#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "light_manager.hpp"
#include "camera_control.hpp"
#include "model.hpp"
#include "load_texture.hpp"
#include "environment_map.hpp"
#include "draw_base_model.hpp"
#include <iostream>

const unsigned int WINDOW_WIDTH = 1080 * 2;
const unsigned int WINDOW_HEIGHT = 720 * 2;

GLFWwindow *initialize_glfw_window()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "scene", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    return window;
}

// lights
glm::vec3 lightPositions[] = {
    glm::vec3(-10.0f, 10.0f, 10.0f),
    glm::vec3(10.0f, 10.0f, 10.0f),
    glm::vec3(-10.0f, -10.0f, 10.0f),
    glm::vec3(10.0f, -10.0f, 10.0f),
};
int nrRows = 7;
int nrColumns = 7;
float spacing = 2.5;

int main()
{
    GLFWwindow *window = initialize_glfw_window();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    Camera camera(window, 45.0f, glm::vec3(0., 0., 10.));

    Shader pbrShader("source/shader/homework_3/pbr.vs", "source/shader/homework_3/pbr_texture_IBL.fs");
    Shader backgroundShader("source/shader/homework_3/background.vs", "source/shader/homework_3/background.fs");

    // 将一个 equirectangular HDR 环境贴图转换为一个立方体贴图，用于PBR环境映射
    unsigned int hdrTexture = load_HDR_texture("source/texture/HDR/kloppenheim_06_puresky_4k.hdr", GL_RGB16F);
    unsigned int envCubemap = convert_equirectangular_to_cubemap(hdrTexture, "source/shader/homework_3/cubemap.vs", "source/shader/homework_3/equirectangular_to_cubemap.fs");
    // 创建了辐照度立方体贴图 irradiance cubemap ，并使用卷积操作来计算漫反射积分，将环境立方体贴图转换为辐照度图，以用于 PBR 中的间接漫反射光照
    unsigned int irradianceMap = generate_irradiance_map(envCubemap, "source/shader/homework_3/cubemap.vs", "source/shader/homework_3/irradiance_convolution.fs");

    pbrShader.use();
    pbrShader.setInt("albedoMap", 0);
    pbrShader.setInt("normalMap", 1);
    pbrShader.setInt("metallicMap", 2);
    pbrShader.setInt("roughnessMap", 3);
    pbrShader.setInt("aoMap", 4);
    pbrShader.setInt("irradianceMap", 5);
    unsigned int albedo = load_texture("source/model/metalgrid2-dx/metalgrid2_basecolor.png");
    unsigned int normal = load_texture("source/model/metalgrid2-dx/metalgrid2_normal-dx.png");
    unsigned int metallic = load_texture("source/model/metalgrid2-dx/metalgrid2_metallic.png");
    unsigned int roughness = load_texture("source/model/metalgrid2-dx/metalgrid2_roughness.png");
    unsigned int ao = load_texture("source/model/metalgrid2-dx/metalgrid2_AO.png");

    backgroundShader.use();
    backgroundShader.setInt("environmentMap", 0);

    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    // light manager
    LightManager light_manager;
    light_manager.add_point_light(lightPositions[0], glm::vec3(150.0f, 150.0f, 150.0f));
    light_manager.add_point_light(lightPositions[1], glm::vec3(150.0f, 150.0f, 150.0f));
    light_manager.add_directional_light(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    light_manager.add_spot_light(lightPositions[3], glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(300.0f, 300.0f, 300.0f));
    light_manager.add_area_light(lightPositions[4], glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(300.0f, 300.0f, 300.0f), 5.0f, 5.0f, 16);
    light_manager.apply_lights(std::make_shared<Shader>(pbrShader));

    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
    {
        camera.compute_matrices_from_inputs(window);
        glm::mat4 view = camera.view;
        glm::mat4 projection = camera.projection;
        glm::vec3 cam_pos = camera.get_pos();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render scene, supplying the convoluted irradiance map to the final shader.
        pbrShader.use();
        pbrShader.setMat4("view", view);
        pbrShader.setMat4("projection", projection);
        pbrShader.setVec3("camPos", cam_pos);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, ao);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

        // 小球
        glm::mat4 model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row)
        {
            // pbrShader.setFloat("metallic", (float)row / (float)nrRows);
            for (int col = 0; col < nrColumns; ++col)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((float)(col - (nrColumns / 2)) * spacing, (float)(row - (nrRows / 2)) * spacing, -2.0f));
                pbrShader.setMat4("model", model);
                pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                render_sphere();
            }
        }

        // 光源
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            glm::vec3 newPos = lightPositions[i];

            model = glm::mat4(1.0f);
            model = glm::translate(model, newPos);
            model = glm::scale(model, glm::vec3(0.5f));
            pbrShader.setMat4("model", model);
            pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            render_sphere();
        }

        // skybox
        backgroundShader.use();
        backgroundShader.setMat4("view", view);
        backgroundShader.setMat4("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
        render_cube();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
