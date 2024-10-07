#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

// 顶点数据
float vertices[] = {
    // 位置
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,

    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f};

unsigned int indices[] = {
    0, 1, 2, 2, 3, 0, // 后面
    4, 5, 6, 6, 7, 4, // 前面
    0, 1, 5, 5, 4, 0, // 底面
    2, 3, 7, 7, 6, 2, // 顶面
    0, 3, 7, 7, 4, 0, // 左面
    1, 2, 6, 6, 5, 1  // 右面
};

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{
    // 初始化 GLFW
    !glfwInit();

    // 设置 OpenGL 版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow *window = glfwCreateWindow(800, 600, "first homework", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化 GLAD
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    unsigned int shaderProgram = LoadShaders("shaderprogram/SimpleVertexShader.vertexshader", "shaderprogram/SimpleFragmentShader.fragmentshader");

    // 创建 VAO, VBO, EBO
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // 绑定 VAO
    glBindVertexArray(VAO);

    // 绑定 VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 绑定 EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 设置顶点属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 开启混合模式，用于透明度
    // glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 处理输入
        processInput(window);

        // 动态修改时间值，传递到顶点着色器
        float timeValue = glfwGetTime();
        int vertexTimeLocation = glGetUniformLocation(shaderProgram, "time");

        // 动态修改透明度（例如根据时间变化）
        float alphaValue = 0.5f + 0.5f * sin(timeValue); // 透明度在 0 到 1 之间变化
        alphaValue = glm::clamp(alphaValue, 0.5f, 1.0f); // 透明度限制在 0.3 到 1.0 之间
        int alphaLocation = glGetUniformLocation(shaderProgram, "alpha");

        // 创建投影矩阵和观察矩阵
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(
            glm::vec3(1.0f, 1.0f, 3.0f), // 相机位置，稍微往上和侧面移动
            glm::vec3(0.0f, 0.0f, 0.0f), // 看向的目标位置，即立方体中心
            glm::vec3(0.0f, 1.0f, 0.0f)  // 世界空间中的上方向，Y轴为正方向
        );
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        // 获取 uniform 位置
        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        // 清除屏幕
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 绘制立方体
        glUseProgram(shaderProgram);
        glUniform1f(vertexTimeLocation, timeValue);                                 // 传递时间值
        glUniform1f(alphaLocation, alphaValue);                                     // 传递透明度值
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));           // 传递模型矩阵
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));             // 传递视角矩阵
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection)); // 传递投影矩阵

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window); // 这是 GLFW 提供的函数，作用是交换前后缓冲区。在现代图形编程中，通常使用 双缓冲技术 来避免屏幕闪烁。
        glfwPollEvents();        // 处理所有的输入和事件，例如键盘、鼠标输入，窗口关闭、窗口大小调整等事件。
    }

    // 释放资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}
