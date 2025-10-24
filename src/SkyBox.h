#pragma once

#include <VAO.h>
#include <VBO.h>
#include <EBO.h>

#include <Shader.h>



#include <string>


struct SkyBox {
	unsigned int ID;

    std::string filepaths[6];

    float vertices[12 * 6] = {
        // right
        0.5f,  -0.5f,  0.5f,
        0.5f,  0.5f,   0.5f,
        0.5f,  0.5f,  -0.5f,
        0.5f,  -0.5f,  -0.5f,

        // left
        -0.5f, -0.5f, -0.5f,
        -0.5f, 0.5f,  -0.5f,
        -0.5f, 0.5f,  0.5f,
        -0.5f, -0.5f, 0.5f,

        // top
        -0.5f,  0.5f, 0.5f,
        -0.5f,  0.5f, -0.5f,
        0.5f,   0.5f, -0.5f,
        0.5f,   0.5f, 0.5f,

        // down
        -0.5f,  -0.5f,  -0.5f,
        -0.5f,  -0.5f,  0.5f,
        0.5f,  -0.5f,   0.5f,
        0.5f,  -0.5f,  -0.5f,

        // front
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,

        // back
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f

    };

    unsigned int indices[6 * 6] = {
        0,1,2,0,2,3,
        4,5,6,4,6,7,
        8,9,10,8,10,11,
        12,13,14,12,14,15,
        16,17,18,16,18,19,
        20,21,22,20,22,23
    };

	VBO vbo;
	VAO vao;
	EBO ebo;

    SkyBox();
	void AddTexture(unsigned int index, const std::string& filepath);
	void Draw(Shader& shader);
    void Bind(unsigned int index = 0);
};