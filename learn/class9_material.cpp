#include <iostream>
#include "glad/glad.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLFW/glfw3.h"

#include "shader.hpp"
#include "load_image.hpp"
#include "objloader.hpp"
#include "camera_control.hpp"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "sphere.hpp"

const unsigned int WINDOW_WIDTH = 1080;
const unsigned int WINDOW_HEIGHT = 720;

glm::vec3 LightPosition_worldspace = glm::vec3(4.0f, 0.0f, 0.0f); // 光源位置
glm::vec3 LightColor = glm::vec3(1, 1, 1);                        // 光源颜色
float LightPower = 1.0f;                                          // 光源强度

float specularStrength = 0.3f; // 镜面反射强度

float vertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

glm::vec3 cubePositions[] = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(2.0f, 5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f, 3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f, 2.0f, -2.5f),
    glm::vec3(1.5f, 0.2f, -1.5f),
    glm::vec3(-1.3f, 1.0f, -1.5f)};

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

    // put the vertices into buffer #0  第一个是顶点放入缓存器中
    glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // put the normals into buffer #2   第三个是将法向量放入缓存器中
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
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CLASS 9 material", NULL, NULL);
    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // 设置视口
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // 顶点数组对象 VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); // 绑定VAO，后续的顶点属性配置和VBO都会储存在这个VAO中

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 顶点缓冲对象 VBO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 法线缓冲对象
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 纹理缓冲对象
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    GLuint texture = load_image("./image/background.bmp");
    GLuint texture2 = load_image("./image/awesomeface.png");

    GLuint VAO_camera;
    setup_vertices(VAO_camera, VBO);

    // 创建着色器对象
    Shader lightingShader("./shaderprogram/class9_vertexshader", "./shaderprogram/class9_fragmentshader");

    Shader lightingShader_c("shaderprogram/homework2_2.vertexshader", "shaderprogram/homework2_2.fragmentshader");

    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
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
        glm::mat4 Projection = camera.ProjectionMatrix;
        glm::mat4 View = camera.ViewMatrix;
        glm::mat4 Model = glm::mat4(1.0f);
        lightingShader.use();
        // lightingShader.setMat4("M", Model);
        lightingShader.setMat4("V", View);
        lightingShader.setMat4("P", Projection);
        lightingShader.setVec3("LightPosition_worldspace", LightPosition_worldspace);

        lightingShader.setInt("material.diffuse", 0);               // 设置漫反射贴图
        lightingShader.setInt("material.specular", 1);              // 设置高光贴图
        lightingShader.setFloat("material.shininess", 5.0f);        // 设置高光系数
        lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f); // 设置镜面反射光属性
        lightingShader.setVec3("light.ambient", 0.1f, 0.1f, 0.1f);  // 设置环境光属性
        lightingShader.setVec3("light.diffuse", 0.4f, 0.4f, 0.4f);  // 设置漫反射光属性
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            Model = glm::mat4(1.0f);
            Model = glm::translate(Model, cubePositions[i]);
            float angle = 20.0f * i;
            Model = glm::rotate(Model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            lightingShader.setMat4("M", Model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 绘制camera球体
        lightingShader_c.use();
        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, LightPosition_worldspace);
        Model = glm::scale(Model, glm::vec3(0.5f, 0.5f, 0.5f));
        glm::mat4 mvp = Projection * View * Model;
        lightingShader_c.setMat4("MVP", Projection * View * Model);
        lightingShader_c.setVec3("mycolor", glm::vec3(1.0f, 1.0f, 1.0f));

        glBindVertexArray(VAO_camera);
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