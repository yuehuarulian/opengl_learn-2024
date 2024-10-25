#include <iostream>
#include "glad/glad.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLFW/glfw3.h"

#include "shader.hpp"
#include "camera_control.hpp"
#include "sphere.hpp"
#include "model.hpp"

const unsigned int WINDOW_WIDTH = 1080;
const unsigned int WINDOW_HEIGHT = 720;

glm::vec3 LightPosition_worldspace = glm::vec3(4.0f, 10.0f, 0.0f); // 光源位置
glm::vec3 LightColor = glm::vec3(1, 1, 1);                         // 光源颜色
float LightPower = 0.7f;                                           // 光源强度

float specularStrength = 0.4f; // 镜面反射强度

Sphere sphere(48);
void setup_vertices(GLuint &VAO, GLuint &VBO)
{
    std::vector<int> ind = sphere.getIndices();         // 球体点的标注
    std::vector<glm::vec3> vert = sphere.getVertices(); // 球上的顶点
    std::vector<glm::vec2> tex = sphere.getTexCoords(); // 纹理坐标
    std::vector<glm::vec3> norm = sphere.getNormals();  // 球上法向量
    std::vector<float> pvalues;                         // vertex positions 顶点位置
    std::vector<float> tvalues;                         // texture coordinates 纹理坐标
    std::vector<float> nvalues;                         // normal vectors 法向量

    int numIndices = sphere.getNumIndices();
    for (int i = 0; i < numIndices; i++)
    { // 把每一个点上的坐标（x,y,z），纹理坐标（s，t），法向量(a,b,c)存储进对应数组
        pvalues.push_back((vert[ind[i]]).x);
        pvalues.push_back((vert[ind[i]]).y);
        pvalues.push_back((vert[ind[i]]).z);
        tvalues.push_back((tex[ind[i]]).s);
        tvalues.push_back((tex[ind[i]]).t);
        nvalues.push_back((norm[ind[i]]).x);
        nvalues.push_back((norm[ind[i]]).y);
        nvalues.push_back((norm[ind[i]]).z);
    }

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

int main()
{
    // 初始化 GLFW
    glfwInit();

    // 创建窗口
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 核心模式

    // 创建窗口 创建OpenGL上下文
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CLASS 4 textured cube", NULL, NULL);
    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // 设置视口
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // 读取模型
    Model model("learn/model/nanosuit/nanosuit.obj");

    GLuint VAO;
    GLuint VBO;
    setup_vertices(VAO, VBO);

    Shader shader("./shaderprogram/class11_vertexshader", "./shaderprogram/class11_fragmentshader");
    Shader shader_c("shaderprogram/homework2_2.vertexshader", "shaderprogram/homework2_2.fragmentshader");

    glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);
    Camera camera(window, 45.0f, glm::vec3(0.0f, 0.0f, 10.0f), glm::pi<float>(), 0.f, 5.0f, 4.0f);
    glfwSwapInterval(1);                                                                            // 垂直同步，参数：在 glfwSwapBuffers 交换缓冲区之前要等待的最小屏幕更新数
    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) // 窗口没有关闭，esc键没有按下
    {
        // 清空屏幕
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        static float rotation_angle = 0.0f;
        rotation_angle += 0.01f;
        LightPosition_worldspace = glm::rotate(glm::mat4(1.0f), 0.01f, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(LightPosition_worldspace, 1.0f);

        camera.computeMatricesFromInputs(window); // 读键盘和鼠标操作，然后计算投影观察矩阵
        glm::mat4 P = camera.ProjectionMatrix;
        glm::mat4 V = camera.ViewMatrix;
        glm::mat4 M = glm::mat4(1.0f);

        shader.use();
        M = glm::translate(M, glm::vec3(0.f, 0.f, 0.f));
        M = glm::rotate(M, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        M = glm::scale(M, glm::vec3(1.f, 1.f, 1.f));
        shader.setMat4("M", M);
        shader.setMat4("V", V);
        shader.setMat4("P", P);
        shader.setVec3("CameraPosition_worldspace", camera.get_camerapos());
        shader.setVec3("pointlight.position", LightPosition_worldspace);
        shader.setVec3("pointlight.ambient", 0.8f, 0.8f, 0.8f);
        shader.setVec3("pointlight.diffuse", 0.8f, 0.8f, 0.8f);
        shader.setVec3("pointlight.specular", 1.0f, 1.0f, 1.0f);
        shader.setFloat("pointlight.constant", 1.0f);
        shader.setFloat("pointlight.linear", 0.09);
        shader.setFloat("pointlight.quadratic", 0.032);
        model.Draw(shader);

        // 绘制camera球体
        shader_c.use();
        M = glm::mat4(1.0f);
        M = glm::translate(M, LightPosition_worldspace);
        M = glm::scale(M, glm::vec3(0.5f, 0.5f, 0.5f));
        shader_c.setMat4("MVP", P * V * M);
        shader_c.setVec3("mycolor", glm::vec3(1.0f, 1.0f, 1.0f));
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINE_LOOP, 0, sphere.getNumIndices());
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDisableVertexAttribArray(0); // 禁用顶点属性数组
    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}