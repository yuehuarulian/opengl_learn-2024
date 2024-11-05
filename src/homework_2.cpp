#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#include "glad/glad.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLFW/glfw3.h"

#include "old_shader.hpp"
#include "load_texture.hpp"
#include "sphere.hpp"
#include "camera_control.hpp"

#define numPlanets 9 // 太阳系 0：太阳 1：水星 2：金星 3：地球 4：火星 5：木星 6：土星 7：天王星 8：海王星

int WINDOW_WIDTH = 1080 * 2;
int WINDOW_HEIGHT = 720 * 2;
const int NUM_VBO = 3;
const float scale = 1e-8f;    // 天体缩放比例
const int day_length = 50;    // 1秒钟对应的天数
const int num_segments = 100; // 圆的细分数量

Sphere sphere(48);
Camera camera;

glm::vec3 LightPosition_worldspace = glm::vec3(0.0f, 0.0f, 0.0f); // 光源位置
glm::vec3 LightColor = glm::vec3(1, 1, 1);                        // 光源颜色
float LightPower = 1.0f;                                          // 光源强度
float specularStrength = 0.1f;                                    // 镜面反射强度

// 行星参数：{半径, 距太阳距离, 公转周期, 纹理文件名}
struct Planet
{
    float radius;
    float distance;
    float orbitPeriod;
    const char *textureFile;
};

const Planet planets[numPlanets] = {
    {69600000.0f, 0.0f, 0.0f, "image/sun.bmp"},                 // 太阳
    {4879000.0f, 57900000.0f, 88.0f, "image/mercury.bmp"},      // 水星
    {12104000.0f, 108200000.0f, 225.0f, "image/venus.bmp"},     // 金星
    {15945000.0f, 150000000.0f, 365.0f, "image/earth.bmp"},     // 地球
    {8340000.0f, 227900000.0f, 687.0f, "image/mars.bmp"},       // 火星
    {69911000.0f, 778500000.0f, 4333.0f, "image/jupiter.bmp"},  // 木星
    {58232000.0f, 1429000000.0f, 10759.0f, "image/saturn.bmp"}, // 土星
    {25362000.0f, 2871000000.0f, 30685.0f, "image/uranus.bmp"}, // 天王星
    {24622000.0f, 4495000000.0f, 60190.0f, "image/neptune.bmp"} // 海王星
};

void init(GLFWwindow *window, GLuint *programID, GLuint *texture)
{
    programID[0] = load_shaders("shaderprogram/homework2_1.vertexshader", "shaderprogram/homework2_1.fragmentshader");
    programID[1] = load_shaders("shaderprogram/homework2_2.vertexshader", "shaderprogram/homework2_2.fragmentshader");
    camera = Camera(window, 45.0f, glm::vec3(0, 3, 20));
    for (int i = 0; i < numPlanets; i++)
        texture[i] = load_texture(planets[i].textureFile); // 加载每个天体的纹理
}

void setup_vertices(GLuint &VAO, GLuint *VBO)
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

    glGenBuffers(NUM_VBO, VBO);
    // put the vertices into buffer #0  第一个是顶点放入缓存器中
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // put the texture coordinates into buffer #1  第二个是将纹理坐标放入缓存器中
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // put the normals into buffer #2   第三个是将法向量放入缓存器中
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void setup_orbit(GLuint &orbitVAO, GLuint &orbitVBO)
{
    // 生成顶点数据
    std::vector<GLfloat> vertices;
    for (int i = 0; i <= num_segments; ++i)
    {
        float theta = 2.0f * GLM_PI * float(i) / float(num_segments);
        float x = cos(theta);
        float z = sin(theta);
        vertices.push_back(x);    // x
        vertices.push_back(0.0f); // y
        vertices.push_back(z);    // z
    }

    // 创建 VAO 和 VBO
    glGenVertexArrays(1, &orbitVAO);
    glBindVertexArray(orbitVAO);

    glGenBuffers(1, &orbitVBO);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

float day(int index)
{
    static double day[numPlanets] = {0};
    static double last_time = glfwGetTime();

    double current_time = glfwGetTime();
    double delta_time = current_time - last_time;
    for (int i = 1; i < numPlanets; i++)
    {
        day[i] += delta_time * day_length;
        if (day[i] >= planets[i].orbitPeriod)
            day[i] -= planets[i].orbitPeriod;
    }

    last_time = current_time; // 更新最后一次时间

    return day[index];
}

void display(GLFWwindow *window, double currentTime, GLuint *programID, GLuint *texture, GLuint *VAO)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.compute_matrices_from_inputs(window);
    glm::mat4 Projection = camera.projection;
    glm::mat4 View = camera.view;

    // 绘制行星
    glUseProgram(programID[0]);
    GLuint Matrix_M = glGetUniformLocation(programID[0], "M");
    GLuint Matrix_V = glGetUniformLocation(programID[0], "V");
    GLuint Matrix_P = glGetUniformLocation(programID[0], "P");

    GLuint LightPositionID = glGetUniformLocation(programID[0], "LightPosition_worldspace");
    GLuint LightColorID = glGetUniformLocation(programID[0], "LightColor");
    GLuint LightPowerID = glGetUniformLocation(programID[0], "LightPower");
    GLuint specularStrengthID = glGetUniformLocation(programID[0], "LightSpecularPower");
    GLuint isSunID = glGetUniformLocation(programID[0], "isSun");
    for (int i = 0; i < numPlanets; i++)
    {
        glBindVertexArray(VAO[0]);

        glm::mat4 Model = glm::mat4(1.0f);

        if (i == 0)
            Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));
        else
        {
            Model = glm::rotate(Model, glm::radians(day(i) / planets[i].orbitPeriod * 360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            Model = glm::translate(Model, glm::vec3(planets[i].distance * scale, 0.0f, 0.0f));
        }
        Model = glm::scale(Model, glm::vec3(planets[i].radius * scale)); // 设置行星的大小

        glUniformMatrix4fv(Matrix_M, 1, GL_FALSE, &Model[0][0]);        // uniform Model
        glUniformMatrix4fv(Matrix_V, 1, GL_FALSE, &View[0][0]);         // uniform View
        glUniformMatrix4fv(Matrix_P, 1, GL_FALSE, &Projection[0][0]);   // uniform Projection
        glUniform3fv(LightPositionID, 1, &LightPosition_worldspace[0]); // 光源位置
        glUniform3fv(LightColorID, 1, &LightColor[0]);                  // 光源颜色
        glUniform1f(LightPowerID, LightPower);                          // 光源强度
        glUniform1f(specularStrengthID, specularStrength);              // 镜面反射强度
        glUniform1i(isSunID, i == 0);                                   // 是否是太阳

        glActiveTexture(GL_TEXTURE0);             // 激活纹理单元
        glBindTexture(GL_TEXTURE_2D, texture[i]); // 绑定纹理
        glDrawArrays(GL_TRIANGLES, 0, sphere.getNumIndices());
    }
    glBindVertexArray(0);

    // 绘制圆形轨道
    glUseProgram(programID[1]);
    GLuint MatrixID = glGetUniformLocation(programID[1], "MVP");
    GLuint ColorID = glGetUniformLocation(programID[1], "mycolor");
    for (int i = 0; i < numPlanets; i++)
    {
        glBindVertexArray(VAO[1]);

        glm::mat4 Model = glm::mat4(1.0f);
        float radius = planets[i].distance * scale;
        Model = glm::scale(Model, glm::vec3(radius, 1.0f, radius));
        glm::mat4 mvp = Projection * View * Model;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(ColorID, 1, glm::value_ptr(color));
        glDrawArrays(GL_LINE_LOOP, 0, num_segments + 1);
    }
    glBindVertexArray(0);
}

int main(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "球体", NULL, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSwapInterval(1); // 垂直同步，参数：在 glfwSwapBuffers 交换缓冲区之前要等待的最小屏幕更新数
    GLuint programID[2];
    GLuint texture[numPlanets];
    GLuint VAO[2];
    GLuint VBO[NUM_VBO];
    GLuint orbitVBO;

    init(window, programID, texture);
    setup_vertices(VAO[0], VBO);
    setup_orbit(VAO[1], orbitVBO);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    static double last_time = glfwGetTime();
    static int frame_count = 0;
    static double fps = 0.0;
    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
    {
        display(window, glfwGetTime(), programID, texture, VAO);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // FPS
        double current_time = glfwGetTime();
        frame_count++;
        if (current_time - last_time >= 1.0)
        {
            fps = frame_count;
            frame_count = 0;
            last_time = current_time;
            std::cout << "FPS: " << fps << std::endl;
        }
    }
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(3, VBO);
    glDeleteProgram(programID[0]);
    glDeleteProgram(programID[1]);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
