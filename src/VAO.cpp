#include "VAO.h"

#include <glad/glad.h>

VAO::VAO() {
    glGenVertexArrays(1, &ID);
}

void VAO::LinkVBO(VBO vbo, int index, int size, int stride, const void* ptr) {
    Bind();
    vbo.Bind();
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, ptr);
    glEnableVertexAttribArray(index);
}

void VAO::Bind() {
    glBindVertexArray(ID);
}

void VAO::Unbind() {
    glBindVertexArray(0);
}

void VAO::Delete() {
    glDeleteVertexArrays(1, &ID);
}