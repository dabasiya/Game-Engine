

#pragma once

#include <glm/glm.hpp>
#include <map>
#include "SubTexture.h"
#include "Texture.h"

#include <string>

struct CharDetail {
	glm::vec2 size;
	glm::vec2 offset;
	float xadvance;
};

struct Fontr {
	Fontr(const char* path, unsigned int fontsize);
	~Fontr();

	std::shared_ptr<Texture> m_Texture;

	unsigned int charwidth[96];

	std::map<char, SubTexture> m_CharMap;

	CharDetail m_CharDetail[96];

	
	// deallocate memory
	void Release();

	// texture width and height;
	unsigned int width;
	unsigned int height;

	unsigned int fontsize;

	float spacewidth = 20.0f;

	void PrintString(std::string text, float x, float y, float z, float scale, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
};

