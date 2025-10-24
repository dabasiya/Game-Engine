#pragma once


class VBO {
private:
    unsigned int ID;
public:
    VBO();
    VBO(int size, void* vertices, unsigned int usage);
    void Bind();
    void Unbind();
    void Delete();
};

