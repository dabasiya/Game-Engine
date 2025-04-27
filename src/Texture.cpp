#include <glad/glad.h>
#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

Texture::Texture(const char* path) {

	this->path = path;

	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* data = stbi_load(path, &width, &height, &channels, 0);


	GLenum internalformat, dataformat;
	if (channels == 4) {
		internalformat = GL_RGBA8;
		dataformat = GL_RGBA;
	}
	else if (channels == 3) {
		internalformat = GL_RGB8;
		dataformat = GL_RGB;
	}


	format = dataformat;


	glCreateTextures(GL_TEXTURE_2D, 1, &ID);
	glTextureStorage2D(ID, 1, internalformat, width, height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTextureSubImage2D(ID, 0, 0, 0, width, height, dataformat, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);

	this->width = width;
	this->height = height;
}

Texture::Texture(unsigned int a_width, unsigned int a_height, int a_internalformat, int a_format, int a_type)
	: format(a_format), type(a_type), width(a_width), height(a_height)
{

	glCreateTextures(GL_TEXTURE_2D, 1, &ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTextureStorage2D(ID, 1, a_internalformat, a_width, a_height);



}

void Texture::Bind(unsigned int id) {
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::SetData(void* data, unsigned int xoffset, unsigned int yoffset, unsigned int a_width, unsigned int a_height) {

	if (a_width == UINT_MAX || a_height == UINT_MAX) {
		a_width = width;
		a_height = height;
	}



	glTextureSubImage2D(ID, 0, xoffset, yoffset, a_width, a_height, format, GL_UNSIGNED_BYTE, data);
}

Texture::~Texture() {
	glDeleteTextures(1, &ID);
}