#include <iostream>
#include "glad/glad.h"

// 处理窗口和键盘输入
#include "GLFW/glfw3.h"

const unsigned int WINDOW_WIDTH = 400;
const unsigned int WINDOW_HEIGHT = 300;
int main()
{
    // 初始化 GLFW
    glfwInit();

    // 创建窗口
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 核心模式

    // 创建窗口 创建OpenGL上下文
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CLASS 1", NULL, NULL);

    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) // 窗口没有关闭，esc键没有按下
    {
        glClear(GL_COLOR_BUFFER_BIT); // 清空屏幕

        // draw something

        // Swap buffers
        glfwSwapBuffers(window); // 这是 GLFW 提供的函数，作用是交换前后缓冲区。在现代图形编程中，通常使用 双缓冲技术 来避免屏幕闪烁。
        // 检查触发事件
        glfwPollEvents(); // 处理所有的输入和事件，例如键盘、鼠标输入，窗口关闭、窗口大小调整等事件。
    }
}