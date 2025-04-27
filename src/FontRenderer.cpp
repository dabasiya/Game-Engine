#include "FontRenderer.h"

#include <iostream>
#include "Renderer2D.h"

#include "window.h"


FontRenderer::FontRenderer(unsigned int fontsize, const std::string& path, unsigned int textureid)
	: fontsize(fontsize)
{
	FT_Init_FreeType(&ft);

	int status = FT_New_Face(ft, path.c_str(), 0, &face);

	if (status != 0)
		std::cout << "Font Not Found!" << std::endl;

	FT_Set_Pixel_Sizes(face, 0, fontsize);

	unsigned int offsetx = 0;



	for (unsigned int i = 0; i < 128; i++) {
		FT_Load_Char(face, i, FT_LOAD_RENDER);

		Character ch = {
		  glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
		  glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
		  (unsigned int)face->glyph->advance.x,
		  offsetx
		};

		characters.insert(std::pair<char, Character>(i, ch));

		offsetx += face->glyph->bitmap.width;
	}

	width = characters[127].offsetx + characters[127].size.x;
	height = fontsize;



	fonttexture = new Texture(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);


	for (auto& ch : characters) {

		unsigned int size = ch.second.size.x * ch.second.size.y * 4;
		unsigned char* charbuffer = new unsigned char[size];

		int status = FT_Load_Char(face, ch.first, FT_LOAD_RENDER);


		for (unsigned i = 0; i < size; i += 4) {
			unsigned int index = (i / 4);
			charbuffer[i] = face->glyph->bitmap.buffer[index];
			charbuffer[i + 1] = face->glyph->bitmap.buffer[index];
			charbuffer[i + 2] = face->glyph->bitmap.buffer[index];
			charbuffer[i + 3] = face->glyph->bitmap.buffer[index];
		}

		fonttexture->SetData(charbuffer, ch.second.offsetx, 0, ch.second.size.x, ch.second.size.y);
		delete charbuffer;

	}


	for (uint32_t c = 0; c < 128; c++) {
		glm::vec2 coord1 = { ((float)characters[c].offsetx) / (float)width, 0.0f };
		glm::vec2 coord2 = { (((float)characters[c].offsetx + characters[c].size.x) / (float)width), ((float)characters[c].size.y / (float)height) };
		chartextures[c] = SubTexture(textureid, coord1, coord2);
	}
}

void FontRenderer::PrintStringui(std::string text, float x, float y, float z, float scale, const glm::vec4& color) {
	float StringWidth = 0;
	float StringHeight = 0;

	for (int i = 0; i < text.length(); i++) {
		StringWidth += ((float)(characters[text[i]].advance >> 6) / fontsize) * scale;
	}

	StringWidth = ((StringWidth / Window::Width) * Window::OrthographicSize * Window::Ratio);

	x -= StringWidth / 2;
	y -= (scale / (2 * Window::Height)) * Window::OrthographicSize;

	float half = (scale / (2 * Window::Height)) * Window::OrthographicSize;

	for (int i = 0; i < text.length(); i++) {

		float sizex = ((float)characters[text[i]].size.x / fontsize) * scale;
		float sizey = ((float)characters[text[i]].size.y / fontsize) * scale;
		float advx = ((float)(characters[text[i]].advance >> 6) / fontsize) * scale;

		float brx = ((float)characters[text[i]].bearing.x / fontsize) * scale;
		float bry = ((float)characters[text[i]].bearing.y / fontsize) * scale;

		float xpos = x + ((brx / (Window::Width)) * Window::Ratio * Window::OrthographicSize);
		//    float ypos = y - ((sizey - bry) / Window::Height) * Window::OrthographicSize;
		float ypos = y + ((bry) / Window::Height) * Window::OrthographicSize;

		float w = (sizex / Window::Width) * Window::Ratio * Window::OrthographicSize;
		float h = (sizey / Window::Height) * Window::OrthographicSize;

		// ypos -= (half/2);

		Renderer2D::DrawQuad({ xpos, ypos, z }, { w, h }, chartextures[text[i]], color, { 0.5f, -0.5f });
		x += (advx / Window::Width) * Window::Ratio * Window::OrthographicSize;
	}
}

void FontRenderer::PrintString(std::string text, float x, float y, float z, float scale, const glm::vec4& color) {
	float StringWidth = 0;
	float StringHeight = 0;

	for (int i = 0; i < text.length(); i++) {
		StringWidth += ((float)(characters[text[i]].advance >> 6) / fontsize) * scale;
	}

	StringWidth = ((StringWidth / Window::Width) * Window::Ratio);

	x -= StringWidth / 2;
	y -= (scale / (2 * Window::Height));

	float half = (scale / (2 * Window::Height));

	for (int i = 0; i < text.length(); i++) {

		float sizex = ((float)characters[text[i]].size.x / fontsize) * scale;
		float sizey = ((float)characters[text[i]].size.y / fontsize) * scale;
		float advx = ((float)(characters[text[i]].advance >> 6) / fontsize) * scale;

		float brx = ((float)characters[text[i]].bearing.x / fontsize) * scale;
		float bry = ((float)characters[text[i]].bearing.y / fontsize) * scale;

		float xpos = x + ((brx / (Window::Width)) * Window::Ratio);
		//    float ypos = y - ((sizey - bry) / Window::Height) * Window::OrthographicSize;
		float ypos = y + ((bry) / Window::Height);

		float w = (sizex / Window::Width) * Window::Ratio;
		float h = (sizey / Window::Height);

		ypos += (half / 2);

		Renderer2D::DrawQuad({ xpos, ypos, z }, { w, h }, chartextures[text[i]], color, { 0.5f, -0.5f });
		x += (advx / Window::Width) * Window::Ratio;
	}
}




void FontRenderer::PrintString(std::string text, float x, float y, float scale, const glm::vec4& color) {
	PrintString(text, x, y, 0.0f, scale, color);
}


// ! Upper commented function removed in future
// and this function have to optimize

void FontRenderer::PrintString(std::string text, int x, int y, float z, int scale, const glm::vec4& color) {

	float xp = ((float)x / Window::Width) * Window::Ratio * Window::OrthographicSize;
	xp = xp - ((Window::OrthographicSize / 2) * Window::Ratio);
	float yp = ((float)y / Window::Height) * Window::OrthographicSize;
	yp = 0 - (yp - (Window::OrthographicSize / 2));


	for (int i = 0; i < text.length(); i++) {

		float sizex = ((float)characters[text[i]].size.x / fontsize) * scale;
		float sizey = ((float)characters[text[i]].size.y / fontsize) * scale;
		float advx = ((float)(characters[text[i]].advance >> 6) / fontsize) * scale;

		float brx = ((float)characters[text[i]].bearing.x / fontsize) * scale;
		float bry = ((float)characters[text[i]].bearing.y / fontsize) * scale;

		float xpos = xp + ((brx / (Window::Width)) * Window::Ratio) + (sizex / (2 * Window::Width)) * Window::Ratio * Window::OrthographicSize;
		float ypos = yp - ((sizey / Window::Height) - (bry / Window::Height)) * Window::OrthographicSize;
		ypos += (sizey / (2 * Window::Height)) * Window::OrthographicSize;

		float w = (sizex / Window::Width) * Window::Ratio * Window::OrthographicSize;
		float h = (sizey / Window::Height) * Window::OrthographicSize;

		Renderer2D::DrawQuad({ xpos, ypos, z }, { w, h }, chartextures[text[i]], color);
		xp += (advx / Window::Width) * Window::OrthographicSize * Window::Ratio;
	}
}

void FontRenderer::PrintString(std::string text, int x, int y, int scale, const glm::vec4& color) {
	PrintString(text, x, y, 0.0f, scale, color);
}


void FontRenderer::Release() {
	delete fonttexture;
}
