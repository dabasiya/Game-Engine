#pragma once

#include <Renderer2D.h>
#include <unordered_map>
#include <FontRenderer.h>


struct FontManager {


	static std::unordered_map<uint32_t, std::shared_ptr<FontRenderer>> m_fontsmap;

	static std::vector<int> m_textureindexes;

	static std::shared_ptr<FontRenderer> GetFont(unsigned int fontsize, const char* fontpath);

	static void ClearFontTextures();
};