#include "FrameBuffer.h"
#include <iostream>

FrameBuffer::FrameBuffer() {
	glGenFramebuffers(1, &ID);
	Bind();
}


void FrameBuffer::Bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, ID);
	if (!colorbuffer) {
		glReadBuffer(GL_NONE);
		glDrawBuffer(GL_NONE);
	}

	else {
		unsigned int m = m_Indexes.size();
		if (m > 0) {
			unsigned int array[10];
			for (unsigned int i = 0; i < m; i++) {
				array[i] = GL_COLOR_ATTACHMENT0 + i;
			}
			glDrawBuffers(m, array);
		}
	}
}

void FrameBuffer::Unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Delete() {
	glDeleteFramebuffers(1, &ID);
}

void FrameBuffer::CheckError() {
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "FrameBuffer Created Successfully!" << std::endl;
	}
	else {
		std::cout << "Failed to Create FrameBuffer!" << std::endl;
	}
}

void FrameBuffer::AddColorAttachment(unsigned int n, const Texture& texture) {
	Bind();
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n, texture.ID, 0);
	colorbuffer = true;
	m_Textures.push_back((Texture*)&texture);
	m_Indexes.push_back(n);
	CheckError();
}

void FrameBuffer::AddDepthAttachment(const Texture& texture, int format) {
	Bind();

	if (texture.m_TextureType == SAMPLER2D)
		glFramebufferTexture(GL_FRAMEBUFFER, format, texture.ID, 0);
	else if (texture.m_TextureType == SAMPLERCUBE)
		glFramebufferTexture(GL_FRAMEBUFFER, format, texture.ID, 0);
	CheckError();
}
