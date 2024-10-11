#include <iostream>
#include "glad/glad.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp" // after <glm/glm.hpp>

#include "GLFW/glfw3.h"
#include "shader.hpp"
#include "error.hpp"
const unsigned int WINDOW_WIDTH = 1080;
const unsigned int WINDOW_HEIGHT = 720;

// clang-format off
static const GLfloat g_vertex_buffer_data[] = {
    -1.0f,-1.0f,-1.0f, // triangle 1 : begin
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, // triangle 1 : end
    1.0f, 1.0f,-1.0f, // triangle 2 : begin
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f, // triangle 2 : end
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f
};

static GLfloat g_color_buffer_data[12*3*3];
// static const GLfloat g_color_buffer_data[] = {
//     0.583f,  0.771f,  0.014f,
//     0.609f,  0.115f,  0.436f,
//     0.327f,  0.483f,  0.844f,
//     0.822f,  0.569f,  0.201f,
//     0.435f,  0.602f,  0.223f,
//     0.310f,  0.747f,  0.185f,
//     0.597f,  0.770f,  0.761f,
//     0.559f,  0.436f,  0.730f,
//     0.359f,  0.583f,  0.152f,
//     0.483f,  0.596f,  0.789f,
//     0.559f,  0.861f,  0.639f,
//     0.195f,  0.548f,  0.859f,
//     0.014f,  0.184f,  0.576f,
//     0.771f,  0.328f,  0.970f,
//     0.406f,  0.615f,  0.116f,
//     0.676f,  0.977f,  0.133f,
//     0.971f,  0.572f,  0.833f,
//     0.140f,  0.616f,  0.489f,
//     0.997f,  0.513f,  0.064f,
//     0.945f,  0.719f,  0.592f,
//     0.543f,  0.021f,  0.978f,
//     0.279f,  0.317f,  0.505f,
//     0.167f,  0.620f,  0.077f,
//     0.347f,  0.857f,  0.137f,
//     0.055f,  0.953f,  0.042f,
//     0.714f,  0.505f,  0.345f,
//     0.783f,  0.290f,  0.734f,
//     0.722f,  0.645f,  0.174f,
//     0.302f,  0.455f,  0.848f,
//     0.225f,  0.587f,  0.040f,
//     0.517f,  0.713f,  0.338f,
//     0.053f,  0.959f,  0.120f,
//     0.393f,  0.621f,  0.362f,
//     0.673f,  0.211f,  0.457f,
//     0.820f,  0.883f,  0.371f,
//     0.982f,  0.099f,  0.879f
// };
// clang-format on

int main()
{
    // 初始化 GLFW
    glfwInit();

    // 创建窗口
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 核心模式

    // 创建窗口 创建OpenGL上下文
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CLASS 4 color cube", NULL, NULL);
    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // 设置视口
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // 顶点数组对象 VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); // 绑定VAO，后续的顶点属性配置和VBO都会储存在这个VAO中

    // 顶点缓冲对象 VBO
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0); // 启用顶点属性数组，和glVertexAttribPointer第一个参数对应

    GLuint VBO2;
    glGenBuffers(1, &VBO2);
    // glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    // glEnableVertexAttribArray(1);

    GLuint programID = LoadShaders("learn/shaderprogram/class4_vertexshader", "learn/shaderprogram/class4_fragmentshader");

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) // 窗口没有关闭，esc键没有按下
    {
        // 生成随机颜色
        float timeValue = glfwGetTime();
        for (int v = 0; v < 12 * 3; v++)
        {
            glm::vec4 vertexPosition_modelspace = glm::vec4(g_vertex_buffer_data[3 * v + 0], g_vertex_buffer_data[3 * v + 1], g_vertex_buffer_data[3 * v + 2], 1.0f);
            g_color_buffer_data[3 * v + 0] = 0.5 + 0.5 * sin(timeValue + vertexPosition_modelspace.x);
            g_color_buffer_data[3 * v + 1] = 0.5 + 0.5 * sin(timeValue + vertexPosition_modelspace.y);
            g_color_buffer_data[3 * v + 2] = 0.5 + 0.5 * sin(timeValue + vertexPosition_modelspace.z);
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glEnableVertexAttribArray(1);

        // 生成MVP矩阵
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
        glm::mat4 View = glm::lookAt(glm::vec3(4, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glm::mat4 Model = glm::mat4(1.0f);
        glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
        GLuint MatrixID = glGetUniformLocation(programID, "MVP");

        // 清空屏幕
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 使用着色器
        glUseProgram(programID);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]); // uniform mvp

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 12 * 3);

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