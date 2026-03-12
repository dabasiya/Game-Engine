#pragma once

#include <gladr.h>
#include "Texture.h"
#include <vector>

struct FrameBuffer {
	unsigned int ID;

	std::vector<Texture*> m_Textures;
	std::vector<unsigned int> m_Indexes;

	bool colorbuffer = false;

	FrameBuffer();
	void AddColorAttachment(unsigned int n, const Texture& texture);
	void AddDepthAttachment(const Texture& texture, int format);
	void Bind();
	void Unbind();
	void Delete();
	void CheckError();
	void Clear();
};
