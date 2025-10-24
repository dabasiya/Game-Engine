
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <Window.h>

#include "FontRenderer2.h"

#include <Renderer2D.h>

Fontr::Fontr(const char* path, unsigned int fontsize)
	:fontsize(fontsize)
{
	unsigned char* ttf_buffer;
	FILE* fontFile;
	fopen_s(&fontFile, path, "rb");
	fseek(fontFile, 0, SEEK_END);
	size_t size = ftell(fontFile);
	fseek(fontFile, 0, SEEK_SET);

	ttf_buffer = (unsigned char*)malloc(size);
	fread(ttf_buffer, 1, size, fontFile);
	fclose(fontFile);

	unsigned char temp_bitmap[32 * 96 * 96]; // atlas texture
	stbtt_bakedchar cdata[96]; // ASCII 32..126

	width = 32 * 96;
	height = 32;

	stbtt_BakeFontBitmap(ttf_buffer, 0, 32.0, temp_bitmap, 32 * 96, 32,
		32, 96, cdata);
	// 32.0 = font size in pixels

	unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char) * 32 * 32 * 96 * 4);

	unsigned int a = 32 * 96 * 32;
	for (unsigned int i = 0; i < a; i++) {
		unsigned char a = temp_bitmap[i];
		unsigned int ind = i * 4;
		data[ind] = a;
		data[ind + 1] = a;
		data[ind + 2] = a;
		data[ind + 3] = a;
	}

	std::shared_ptr<Texture> m_texture = std::make_shared<Texture>(32 * 96, 32, GL_RGBA8, GL_RGBA, 0);

	m_texture->SetData((void*)data, GL_RGBA8, GL_RGBA);

	Renderer2D::textures[10] = m_texture;

	for (unsigned int i = 0; i < 96; i++) {
		SubTexture a;
		glm::vec2 c1, c2;

		c1.x = (float)cdata[i].x0 / width;
		c2.x = (float)cdata[i].x1 / width;
		c1.y = (float)cdata[i].y0 / height;
		c2.y = (float)cdata[i].y1 / height;

		m_CharDetail[i].size = { cdata[i].x1 - cdata[i].x0, cdata[i].y1 - cdata[i].y0 };
		m_CharDetail[i].offset = { cdata[i].xoff, cdata[i].yoff };
		m_CharDetail[i].xadvance = cdata[i].xadvance;

		charwidth[i] = cdata[i].x1 - cdata[i].x0;

		a.coords1 = c1;
		a.coords2 = c2;
		a.index = 10;

		m_CharMap.insert(std::pair<char, SubTexture>((char)i + 32, a));
	}
}

Fontr::~Fontr() {

}


void Fontr::PrintString(std::string text, float x, float y, float z, float scale, const glm::vec4& color) {
	float StringWidth = 0;
	float StringHeight = 0;

	for (int i = 0; i < text.length(); i++) {
		if (text[i] < 32 || text[i] > 126) {
			StringWidth += (m_CharDetail[text[i] - 32].size.x / Window::Width) * Window::Ratio * Window::OrthographicSize * scale / fontsize;
			continue;
		}
		unsigned int index = (int)text[i] - 32;
		StringWidth += (float)(m_CharDetail[index].size.x / Window::Width) * scale / fontsize;
	}

	StringWidth = ((StringWidth / Window::Width) * Window::OrthographicSize * Window::Ratio);

	x -= StringWidth / 2;
	y -= (scale / (2 * Window::Height)) * Window::OrthographicSize;

	float half = (scale / (2 * Window::Height)) * Window::OrthographicSize;

	for (int i = 0; i < text.length(); i++) {

		if (text[i] < 32 || text[i] > 126) {
			x += (scale / fontsize) * (spacewidth / Window::Width);
			continue;
			x += (float)(m_CharDetail[text[i]-32].xadvance / Window::Width) * scale/fontsize;
		}

		float yoff = (-m_CharDetail[text[i]-32].offset.y / Window::Height) * Window::OrthographicSize * scale / fontsize;
		float xoff = (m_CharDetail[text[i]-32].offset.x / Window::Width) * Window::Ratio * Window::OrthographicSize * scale / fontsize;

		float w = (m_CharDetail[text[i] - 32].size.x / Window::Width) * Window::Ratio * Window::OrthographicSize * scale / fontsize;
		float h = (m_CharDetail[text[i]-32].size.y / Window::Height) * Window::OrthographicSize * scale / fontsize;

		// ypos -= (half/2);
		Renderer2D::DrawQuad({ x+xoff, y+yoff, z }, { w, h }, m_CharMap[text[i]], color, { 0.5f, -0.5f });
		x += (m_CharDetail[text[i]-32].xadvance / Window::Width) * Window::Ratio * Window::OrthographicSize * scale / fontsize;
	}
}

