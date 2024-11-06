#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "camera_control.hpp"
#include "model.hpp"
#include "load_texture.hpp"
#include "environment_map.hpp"
#include "draw_base_model.hpp"
#include <iostream>

// settings
const unsigned int WINDOW_WIDTH = 1080 * 2;
const unsigned int WINDOW_HEIGHT = 720 * 2;

// camera
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 初始化窗口和 OpenGL 上下文
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
glm::vec3 lightColors[] = {
    glm::vec3(300.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f, 300.0f)};
int nrRows = 7;
int nrColumns = 7;
float spacing = 2.5;

int main()
{
    GLFWwindow *window = initialize_glfw_window();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    Camera camera(window, 45.0f, glm::vec3(0., 0., 10.));

    Shader pbrShader("source/shader/class15/pbr.vs", "source/shader/class15/pbr.fs");
    Shader backgroundShader("source/shader/class16/background.vs", "source/shader/class16/background.fs");

    // 将一个 equirectangular HDR 环境贴图转换为一个立方体贴图 (cubemap)，用于物理渲染（PBR）环境映射
    unsigned int hdrTexture = load_HDR_texture("source/texture/HDR/kloppenheim_06_puresky_4k.hdr", GL_RGB16F);
    unsigned int envCubemap = convert_equirectangular_to_cubemap(hdrTexture, "source/shader/class16/cubemap.vs", "source/shader/class16/equirectangular_to_cubemap.fs");

    // 创建了辐照度立方体贴图 (irradiance cubemap)，并使用卷积操作来计算漫反射积分，将环境立方体贴图转换为辐照度图，以用于物理基础渲染 (PBR) 中的间接漫反射光照
    unsigned int irradianceMap = generate_irradiance_map(envCubemap, "source/shader/class16/cubemap.vs", "source/shader/class16/irradiance_convolution.fs");

    pbrShader.use();
    pbrShader.setInt("irradianceMap", 0);
    pbrShader.setVec3("albedo", 0.5f, 0.0f, 0.0f);
    pbrShader.setFloat("ao", 1.0f);
    // pbrShader.setInt("albedoMap", 0);
    // pbrShader.setInt("normalMap", 1);
    // pbrShader.setInt("metallicMap", 2);
    // pbrShader.setInt("roughnessMap", 3);
    // pbrShader.setInt("aoMap", 4);
    // unsigned int albedo = load_texture("source/model/metalgrid2-dx/metalgrid2_basecolor.png");
    // unsigned int normal = load_texture("source/model/metalgrid2-dx/metalgrid2_normal-dx.png");
    // unsigned int metallic = load_texture("source/model/metalgrid2-dx/metalgrid2_metallic.png");
    // unsigned int roughness = load_texture("source/model//metalgrid2-dx/metalgrid2_roughness.png");
    // unsigned int ao = load_texture("source/model/metalgrid2-dx/metalgrid2_AO.png");

    backgroundShader.use();
    backgroundShader.setInt("environmentMap", 0);

    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // camera
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

        // bind pre-computed IBL data
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

        // render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
        glm::mat4 model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row)
        {
            pbrShader.setFloat("metallic", (float)row / (float)nrRows);
            for (int col = 0; col < nrColumns; ++col)
            {
                pbrShader.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                                                  (float)(col - (nrColumns / 2)) * spacing,
                                                  (float)(row - (nrRows / 2)) * spacing,
                                                  -2.0f));
                pbrShader.setMat4("model", model);
                pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                render_sphere();
            }
        }

        // render light source (simply re-render sphere at light positions)
        // this looks a bit off as we use the same shader, but it'll make their positions obvious and
        // keeps the codeprint small.
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            newPos = lightPositions[i];
            pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
            pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

            model = glm::mat4(1.0f);
            model = glm::translate(model, newPos);
            model = glm::scale(model, glm::vec3(0.5f));
            pbrShader.setMat4("model", model);
            pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            render_sphere();
        }

        // render skybox (render as last to prevent overdraw)
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
