#include <iostream>
#include <vector>
#include <string>
// #include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "camera_control.hpp"
#include "model.hpp"
#include "load_texture.hpp"
#include "draw_base_model.hpp"
#include "skybox.hpp"
#include "light_manager.hpp"
#include "renderable_model.hpp"

// settings
const unsigned int WINDOW_WIDTH = 1080 * 2;
const unsigned int WINDOW_HEIGHT = 720 * 2;

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

vector<std::string> faces{
    "source/skybox/sky/right.jpg",
    "source/skybox/sky/left.jpg",
    "source/skybox/sky/top.jpg",
    "source/skybox/sky/bottom.jpg",
    "source/skybox/sky/front.jpg",
    "source/skybox/sky/back.jpg"};

// light
glm::vec3 lightPositions[] = {
    glm::vec3(0.0f, 0.0f, 10.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
};
glm::vec3 lightColors[] = {
    glm::vec3(150.0f, 150.0f, 150.0f),
    glm::vec3(150.0f, 150.0f, 150.0f),
};
int nrRows = 7;
int nrColumns = 7;
float spacing = 2.5;

int main()
{
    GLFWwindow *window = initialize_glfw_window();

    Camera camera(window, 45.0f, glm::vec3(0., 0., 10.));

    Shader shader("source/shader/class15/pbr.vs", "source/shader/class15/pbr_texture.fs");
    Skybox skybox(faces, "source/shader/class14/skybox.vs", "source/shader/class14/skybox.fs");

    shader.use();
    shader.setInt("albedoMap", 0);
    shader.setInt("normalMap", 1);
    shader.setInt("metallicMap", 2);
    shader.setInt("roughnessMap", 3);
    shader.setInt("aoMap", 4);

    unsigned int albedo = load_texture("source/model/metalgrid2-dx/metalgrid2_basecolor.png");
    unsigned int normal = load_texture("source/model/metalgrid2-dx/metalgrid2_normal-dx.png");
    unsigned int metallic = load_texture("source/model/metalgrid2-dx/metalgrid2_metallic.png");
    unsigned int roughness = load_texture("source/model/metalgrid2-dx/metalgrid2_roughness.png");
    unsigned int ao = load_texture("source/model/metalgrid2-dx/metalgrid2_AO.png");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
    {
        camera.compute_matrices_from_inputs(window);
        glm::mat4 view = camera.view;
        glm::mat4 projection = camera.projection;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.render(view, projection);

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("camPos", camera.get_pos());

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

        glm::mat4 model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row)
        {
            // shader.setFloat("metallic", (float)row / (float)nrRows);
            for (int col = 0; col < nrColumns; ++col)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                                                  (col - (nrColumns / 2)) * spacing,
                                                  (row - (nrRows / 2)) * spacing,
                                                  0.0f));
                shader.setMat4("model", model);
                shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                render_sphere();
            }
        }

        // 画光源
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            newPos = lightPositions[i];
            shader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
            shader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

            model = glm::mat4(1.0f);
            model = glm::translate(model, newPos);
            model = glm::scale(model, glm::vec3(0.5f));
            shader.setMat4("model", model);
            shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            render_sphere();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
