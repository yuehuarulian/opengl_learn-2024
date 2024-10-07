#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp" // after <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "GLFW/glfw3.h"

#include "shader.hpp"

// 处理窗口和键盘输入
#include "GLFW/glfw3.h"

const unsigned int WINDOW_WIDTH = 1080;
const unsigned int WINDOW_HEIGHT = 720;

// clang-format off
static const GLfloat g_vertex_buffer_data[] = {
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     0.0f,  1.0f, 0.0f,
};
// clang-format on

void checkGLError(const char *functionName)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error in " << functionName << ": " << err << std::endl;
    }
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
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CLASS 3", NULL, NULL);
    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // 设置视口
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    GLuint VAO;                 // 顶点数组对象 VAO
    glGenVertexArrays(1, &VAO); // 生成VAO
    glBindVertexArray(VAO);     // 绑定VAO，后续的顶点属性配置和VBO都会储存在这个VAO中

    // 顶点缓冲对象 VBO
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // 配置顶点属性指针，这会被 VAO "记住"
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0); // 启用顶点属性数组，和glVertexAttribPointer第一个参数对应

    GLuint programID = LoadShaders("learn/SimpleVertexShader.vertexshader", "learn/SimpleFragmentShader.fragmentshader");
    checkGLError("LoadShaders");

    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    // glm::mat4 Projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f); // In world coordinates
    glm::mat4 View = glm::lookAt(glm::vec3(4, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 Model = glm::mat4(1.0f);
    glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
    // uniform mvp
    glUseProgram(programID);
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
    checkGLError("glUniformMatrix4fv");

    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) // 窗口没有关闭，esc键没有按下
    {
        // 清空屏幕
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 使用着色器
        glUseProgram(programID);

        // draw something
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window); // 这是 GLFW 提供的函数，作用是交换前后缓冲区。在现代图形编程中，通常使用 双缓冲技术 来避免屏幕闪烁。
        glfwPollEvents();        // 处理所有的输入和事件，例如键盘、鼠标输入，窗口关闭、窗口大小调整等事件。
    }
    // glDisableVertexAttribArray(0); // 禁用顶点属性数组
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

int main1()
{
    glm::vec4 OriginalVector(10.0f, 10.0f, 10.0f, 1.0f);

    /********平移矩阵********/
    glm::mat4 TranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 0.0f));
    glm::vec4 transformedVector = TranslationMatrix * OriginalVector; // guess the result
    std::cout << "Transformed Vector: ("
              << transformedVector.x << ", "
              << transformedVector.y << ", "
              << transformedVector.z << ", "
              << transformedVector.w << ")" << std::endl;

    /********单位矩阵********/
    glm::mat4 myIdentityMatrix = glm::mat4(1.0);

    /********缩放矩阵********/
    glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
    transformedVector = ScaleMatrix * OriginalVector;
    std::cout << "Transformed Vector: ("
              << transformedVector.x << ", "
              << transformedVector.y << ", "
              << transformedVector.z << ", "
              << transformedVector.w << ")" << std::endl;

    /********旋转矩阵********/
    glm::vec3 myRotationAxis(1., 0., 0.);
    float angle_in_radians = glm::radians(180.0f);
    glm::mat4 RotationMatrix = glm::rotate(angle_in_radians, myRotationAxis);
    std::cout << "RotationMatrix: ("
              << RotationMatrix[0][0] << ", " << RotationMatrix[0][1] << ", " << RotationMatrix[0][2] << ", " << RotationMatrix[0][3] << '\n'
              << RotationMatrix[1][0] << ", " << RotationMatrix[1][1] << ", " << RotationMatrix[1][2] << ", " << RotationMatrix[1][3] << '\n'
              << RotationMatrix[2][0] << ", " << RotationMatrix[2][1] << ", " << RotationMatrix[2][2] << ", " << RotationMatrix[2][3] << '\n'
              << RotationMatrix[3][0] << ", " << RotationMatrix[3][1] << ", " << RotationMatrix[3][2] << ", " << RotationMatrix[3][3] << ")" << std::endl;

    /********累积变换********/
    glm::vec4 TransformedVector = TranslationMatrix * RotationMatrix * ScaleMatrix * OriginalVector;
    std::cout << "Transformed Vector: ("
              << TransformedVector.x << ", "
              << TransformedVector.y << ", "
              << TransformedVector.z << ", "
              << TransformedVector.w << ")" << std::endl;

    /*
        glm::mat4 CameraMatrix = glm::LookAt(
            cameraPosition, // the position of your camera, in world space
            cameraTarget,   // where you want to look at, in world space
            upVector        // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
        );
    */

    // Generates a really hard-to-read matrix, but a normal, standard 4x4 matrix nonetheless
    // glm::mat4 projectionMatrix = glm::perspective(
    //     glm::radians(FoV), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90&deg; (extra wide) and 30&deg; (quite zoomed in)
    //     4.0f / 3.0f,       // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
    //     0.1f,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
    //     100.0f             // Far clipping plane. Keep as little as possible.
    // );

    /********从摄像机空间到齐次坐空间（Homogeneous Space）********/

    /********复合变换：模型观察投影矩阵（MVP）********/
    // glm::mat4 MVPmatrix = projection * view * model;

    // Or, for an ortho camera :
    // glm::mat4 Projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f); // In world coordinates
    return 0;
}
