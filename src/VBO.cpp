#include "VBO.h"
#include <glad/glad.h>

VBO::VBO(int size, void* vertices, unsigned int usage) {
    glGenBuffers(1, &ID);
    Bind();
    glBufferData(GL_ARRAY_BUFFER, size, vertices, usage);
}

VBO::VBO() {

}

void VBO::Bind() {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::Unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Delete() {
    glDeleteBuffers(1, &ID);
}