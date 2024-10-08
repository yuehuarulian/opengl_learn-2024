#ifndef LOAD_BMP_HPP
#define LOAD_BMP_HPP
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint load_image(const char *imagepath)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // 为当前绑定的纹理对象设置环绕、过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 加载并生成纹理
    int width, height, nrChannels;
    unsigned char *data = stbi_load(imagepath, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return texture;

    // unsigned char header[54]; // Each BMP file begins by a 54-bytes header
    // unsigned int dataPos;     // Position in the file where the actual data begins
    // unsigned int width, height;
    // unsigned int imageSize; // = width*height*3
    // // Actual RGB data
    // unsigned char *data;

    // // Open the file
    // FILE *file = fopen(imagepath, "rb");
    // if (!file)
    // {
    //     printf("Image could not be opened\n");
    //     return 0;
    // }

    // if (fread(header, 1, 54, file) != 54)
    // { // If not 54 bytes read : problem
    //     printf("Not a correct BMP file\n");
    //     return false;
    // }

    // if (header[0] != 'B' || header[1] != 'M')
    // {
    //     printf("Not a correct BMP file\n");
    //     return 0;
    // }

    // // Read ints from the byte array
    // dataPos = *(int *)&(header[0x0A]);
    // imageSize = *(int *)&(header[0x22]);
    // width = *(int *)&(header[0x12]);
    // height = *(int *)&(header[0x16]);

    // // Some BMP files are misformatted, guess missing information
    // if (imageSize == 0)
    //     imageSize = width * height * 3; // 3 : one byte for each Red, Green and Blue component
    // if (dataPos == 0)
    //     dataPos = 54; // The BMP header is done that way

    // // Create a buffer
    // data = new unsigned char[imageSize];

    // // Read the actual data from the file into the buffer
    // fread(data, 1, imageSize, file);

    // // Everything is in memory now, the file can be closed
    // fclose(file);

    // // Create one OpenGL texture
    // GLuint textureID;
    // glGenTextures(1, &textureID);
    // // "Bind" the newly created texture : all future texture functions will modify this texture
    // glBindTexture(GL_TEXTURE_2D, textureID);
    // // Give the image to OpenGL
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    // // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // // When MAGnifying the image (no bigger mipmap available), use LINEAR filtering
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // // When MINifying the image, use a LINEAR blend of two mipmaps, each filtered LINEARLY too
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // // Generate mipmaps, by the way.
    // glGenerateMipmap(GL_TEXTURE_2D);

    // delete[] data;
    // printf("Texture loaded\n");
    // return textureID;
}

#endif