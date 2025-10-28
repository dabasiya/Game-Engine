#include <glad/glad.h>
#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

Texture::Texture(const char* path) {

	this->path = path;

	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

	if (!data) {
		std::cout << path << " not found" << std::endl;
	}


	this->width = width;
	this->height = height;


	GLint internalformat = GL_RGBA8, dataformat = GL_RGBA;
	if (channels == 4) {
		internalformat = GL_RGBA8;
		dataformat = GL_RGBA;
	}
	else if (channels == 3) {
		internalformat = GL_RGB8;
		dataformat = GL_RGB;
	}


	format = dataformat;


	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);


	stbi_image_free(data);
}


Texture::Texture(const char* path, const std::string& type) {
	this->path = path;
	this->type = type;

	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

	if (!data) {
		std::cout << path << " not found" << std::endl;
	}

	this->width = width;
	this->height = height;


	GLint internalformat = GL_RGBA8, dataformat = GL_RGBA;
	if (channels == 4) {
		internalformat = GL_RGBA8;
		dataformat = GL_RGBA;
	}
	else if (channels == 3) {
		internalformat = GL_RGB8;
		dataformat = GL_RGB;
	}


	format = dataformat;


	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
}


Texture::Texture(unsigned int a_width, unsigned int a_height, int a_internalformat, int a_format, int a_type)
	: format(a_format), atype(a_type), width(a_width), height(a_height)
{

	glGenTextures(1, &ID);

	glBindTexture(GL_TEXTURE_2D, ID);

	glTextureStorage2D(ID, 1, a_internalformat, width, height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
}

Texture::Texture(unsigned int a_width, unsigned int a_height, int a_internalformat) {

	width = a_width;
	height = a_height;

	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	for (unsigned int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, a_internalformat, a_width, a_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	m_TextureType = SAMPLERCUBE;
}

void Texture::Bind(unsigned int id) {
	glActiveTexture(GL_TEXTURE0 + id);
	if(m_TextureType == SAMPLER2D)
		glBindTexture(GL_TEXTURE_2D, ID);
	else 
		glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
}

void Texture::SetData(void* data, int a_internalformat, int format) {
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexImage2D(GL_TEXTURE_2D, 0, a_internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

}


void Texture::SetData(void* data, unsigned int xoffset, unsigned int yoffset, unsigned int a_width, unsigned int a_height) {

	if (a_width == UINT_MAX || a_height == UINT_MAX) {
		a_width = width;
		a_height = height;
	}



	glTextureSubImage2D(ID, 0, xoffset, yoffset, a_width, a_height, format, GL_UNSIGNED_BYTE, data);
}

void Texture::Bind(Shader& shader, const std::string& name, unsigned int index) {
	shader.Bind();
	glActiveTexture(GL_TEXTURE0 + index);
	if (m_TextureType == SAMPLER2D)
		glBindTexture(GL_TEXTURE_2D, ID);
	else
		glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	shader.SetInt(name.c_str(), (int)index);
}

Texture::~Texture() {
	glDeleteTextures(1, &ID);
}