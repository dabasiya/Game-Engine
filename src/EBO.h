#pragma once

class EBO {
private:
    unsigned int ID;
public:
    EBO();
    EBO(int size, void* indices);
    void Bind();
    void Unbind();
    void Delete();
};

