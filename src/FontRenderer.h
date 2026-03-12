

#pragma once


#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <map>
#include "SubTexture.h"
#include "Texture.h"

#include <string>

struct Character {
	glm::ivec2 size;
	glm::ivec2 bearing;
	unsigned int advance;
	unsigned int offsetx;
};

struct FontRenderer {

	unsigned int textureindex;

	FontRenderer(unsigned int fontsize, const std::string& path, unsigned int textureid);
	~FontRenderer();


	// for gl positions
	void PrintString(const char* text, float x, float y, float scale, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	void PrintString(const char* text, float x, float y, float z, float scale, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	// for window positions
	void PrintString(const char* text, int x, int y, int scale, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	void PrintString(const char* text, int x, int y, float z, int scale, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	void PrintStringui(const char* text, float x, float y, float z, float scale, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f }, float xbegin = false);


	// deallocate memory
	void Release();

	// other data

	int fontsize;

	// texture sheet for storing all characters

	std::shared_ptr<Texture> fonttexture;


	// texture width and height
	unsigned int width;
	unsigned int height;


	FT_Library ft;
	FT_Face face;

	std::map<char, Character> characters;

	// all character subtextures

	SubTexture chartextures[128];


};


