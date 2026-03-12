#pragma once


#include <climits>
#include <string>

#include "Shader.h"

enum TextureType {
	SAMPLER2D = 0,
	SAMPLERCUBE = 1
};

struct Texture {

	// unique id for texture
	unsigned int ID;

	// data

	unsigned int width = 0, height = 0;
	int format, atype;

	std::string type;

	TextureType m_TextureType = SAMPLER2D;

	// texture constructors

	std::string path;

	Texture(const char* path);
	Texture(unsigned int a_width, unsigned int a_height, int a_internalformat, int a_format, int a_type, bool clamp_border = true); 
	Texture(unsigned int a_width, unsigned int a_height, int a_internalformat);
	Texture(const char* path, const std::string& type);

	~Texture();

	void Bind(unsigned int id = 0);

	void SetData(void* data, int a_internalformat, int format);
	void SetData(void* data, unsigned int xoffset = 0, unsigned int yoffset = 0, unsigned int a_width = UINT_MAX, unsigned int a_height = UINT_MAX);

	void Bind(Shader& shader, const std::string& name, unsigned int index);

};
