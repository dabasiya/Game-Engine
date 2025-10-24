#pragma once

#include "VBO.h"


class VAO {
private:
    unsigned int ID;
public:
    VAO();
    void LinkVBO(VBO vbo, int index, int size, int stride, const void* ptr);
    void Bind();
    void Unbind();
    void Delete();
};

