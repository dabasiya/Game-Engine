#pragma once


#include <climits>
#include <string>

struct Texture {

	// unique id for texture
	unsigned int ID;

	// data

	unsigned int width = 0, height = 0;
	int format, type;

	// texture constructors

	std::string path;

	Texture(const char* path);
	Texture(unsigned int a_width, unsigned int a_height, int a_internalformat, int a_format, int a_type);
	~Texture();

	void Bind(unsigned int id = 0);


	void SetData(void* data, unsigned int xoffset = 0, unsigned int yoffset = 0, unsigned int a_width = UINT_MAX, unsigned int a_height = UINT_MAX);


};
