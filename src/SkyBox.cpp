#include <SkyBox.h>

#include <glad/glad.h>

#include <stb_image.h>


#define SKYBOX_RIGHT 0
#define SKYBOX_LEFT 1
#define SKYBOX_TOP 2
#define SKYBOX_BOTTOM 3
#define SKYBOX_FRONT 4
#define SKYBOX_BACK 5

SkyBox::SkyBox() {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    vbo = VBO(sizeof(vertices), vertices, GL_STATIC_DRAW);
    vao.LinkVBO(vbo, 0, 3, 3 * sizeof(float), 0);
    ebo = EBO(sizeof(indices), indices);


}

void SkyBox::Bind(unsigned int index) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
}


void SkyBox::AddTexture(unsigned int index, const std::string& filepath) {

    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

    filepaths[index] = filepath;

    stbi_set_flip_vertically_on_load(false);

    int width, height, channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
}


void SkyBox::Draw(Shader& shader) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

    shader.Bind();

    vao.Bind();
    ebo.Bind();

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}