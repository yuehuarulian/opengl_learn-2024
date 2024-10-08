#include <iostream>
#include "glad/glad.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLFW/glfw3.h"

#include "shader.hpp"
#include "load_bmp.hpp"
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
float LightPower = 0.7f;                                          // 光源强度

float specularStrength = 0.4f; // 镜面反射强度

Sphere sphere(48);
void setup_vertices(GLuint &VAO, GLuint &VBO, GLuint &normal)
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
    // put the vertices into buffer #0  第一个是顶点放入缓存器中
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // put the normals into buffer #2   第三个是将法向量放入缓存器中
    glBindBuffer(GL_ARRAY_BUFFER, normal);
    glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);
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
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    // bool res = loadOBJ("learn/model/monkey.obj", vertices, uvs, normals);
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile("learn/model/cube.obj", aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene)
    {
        std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
        return -1;
    }
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            aiVector3D pos = mesh->mVertices[j];
            aiVector3D normal = mesh->mNormals[j];
            aiVector3D uv = mesh->mTextureCoords[0][j];
            vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
            normals.push_back(glm::vec3(normal.x, normal.y, normal.z));
            uvs.push_back(glm::vec2(uv.x, uv.y));
        }
    }
    // 顶点数组对象 VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); // 绑定VAO，后续的顶点属性配置和VBO都会储存在这个VAO中

    // 顶点缓冲对象 VBO
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0); // 启用顶点属性数组，和glVertexAttribPointer第一个参数对应

    // 纹理缓冲对象
    GLuint texture = loadBMP_custom("./image/background.bmp");
    glGenBuffers(1, &texture);
    glBindBuffer(GL_ARRAY_BUFFER, texture);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);

    // 法线缓冲对象
    GLuint normal;
    glGenBuffers(1, &normal);
    glBindBuffer(GL_ARRAY_BUFFER, normal);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(2);

    GLuint VAO_camera;
    setup_vertices(VAO_camera, VBO, normal);

    GLuint programID = LoadShaders("./shaderprogram/class8_vertexshader", "./shaderprogram/class8_fragmentshader");
    GLuint programID_camera = LoadShaders("shaderprogram/homework2_2.vertexshader", "shaderprogram/homework2_2.fragmentshader");

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    // glDepthFunc(GL_LESS);
    Camera camera(window, 45.0f, glm::vec3(0.0f, 0.0f, 10.0f), glm::pi<float>(), 0.f, 5.0f, 4.0f);
    glfwSwapInterval(1);                                                                            // 垂直同步，参数：在 glfwSwapBuffers 交换缓冲区之前要等待的最小屏幕更新数
    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) // 窗口没有关闭，esc键没有按下
    {
        static float rotation_angle = 0.0f;
        rotation_angle += 0.01f;
        // LightPosition_worldspace = glm::rotate(glm::mat4(1.0f), rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(LightPosition_worldspace, 1.0f);
        LightPosition_worldspace = glm::rotate(glm::mat4(1.0f), 0.01f, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(LightPosition_worldspace, 1.0f);

        camera.computeMatricesFromInputs(window); // 读键盘和鼠标操作，然后计算投影观察矩阵
        glm::mat4 Projection = camera.ProjectionMatrix;
        glm::mat4 View = camera.ViewMatrix;
        glm::mat4 Model = glm::mat4(1.0f);
        GLuint Matrix_M = glGetUniformLocation(programID, "M");
        GLuint Matrix_V = glGetUniformLocation(programID, "V");
        GLuint Matrix_P = glGetUniformLocation(programID, "P");

        GLuint LightPositionID = glGetUniformLocation(programID, "LightPosition_worldspace");
        GLuint LightColorID = glGetUniformLocation(programID, "LightColor");
        GLuint LightPowerID = glGetUniformLocation(programID, "LightPower");
        GLuint specularStrengthID = glGetUniformLocation(programID, "LightSpecularPower");

        // 清空屏幕
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 绘制模型
        glUseProgram(programID);
        glUniformMatrix4fv(Matrix_M, 1, GL_FALSE, &Model[0][0]);        // uniform Model
        glUniformMatrix4fv(Matrix_V, 1, GL_FALSE, &View[0][0]);         // uniform View
        glUniformMatrix4fv(Matrix_P, 1, GL_FALSE, &Projection[0][0]);   // uniform Projection
        glUniform3fv(LightPositionID, 1, &LightPosition_worldspace[0]); // 光源位置
        glUniform3fv(LightColorID, 1, &LightColor[0]);                  // 光源颜色
        glUniform1f(LightPowerID, LightPower);                          // 光源强度
        glUniform1f(specularStrengthID, specularStrength);              // 镜面反射强度
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 12 * 3);

        // 绘制camera球体
        glUseProgram(programID_camera);
        GLuint MatrixID = glGetUniformLocation(programID_camera, "MVP");
        GLuint ColorID = glGetUniformLocation(programID_camera, "mycolor");
        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, LightPosition_worldspace);
        Model = glm::scale(Model, glm::vec3(0.5f, 0.5f, 0.5f));
        glm::mat4 mvp = Projection * View * Model;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(ColorID, 1, glm::value_ptr(color));
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