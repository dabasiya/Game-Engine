#include <FontManager.h>
#include <iostream>

std::unordered_map<uint32_t, std::shared_ptr<FontRenderer>> FontManager::m_fontsmap;

std::vector<int> FontManager::m_textureindexes;

std::shared_ptr<FontRenderer> FontManager::GetFont(unsigned int fontsize, const char* filepath) {

	char buffer[128];
	snprintf(buffer, 128, "%s%d", filepath, fontsize);
	uint32_t hash = Renderer2D::HashString(buffer);
	if (m_fontsmap.find(hash) == m_fontsmap.end()) {
		unsigned int texindex = Renderer2D::GetUnusedTextureIndex();
		std::shared_ptr<FontRenderer> af = std::make_shared<FontRenderer>(fontsize, filepath, texindex);
		m_textureindexes.push_back(texindex);
		m_fontsmap[hash] = af;
		Renderer2D::SetTexture(af->fonttexture, texindex);
		return af;
	}

	auto af = m_fontsmap[hash];
	Renderer2D::SetTexture(af->fonttexture, af->textureindex);
	m_textureindexes.push_back(af->textureindex);
	return af;
}


void FontManager::ClearFontTextures() {

}