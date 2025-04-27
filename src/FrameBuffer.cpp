#include "FrameBuffer.h"
#include <iostream>

FrameBuffer::FrameBuffer() {
  glGenFramebuffers(1, &ID);
  Bind();
}


void FrameBuffer::Bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, ID);
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
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n, GL_TEXTURE_2D, texture.ID, 0);
}

void FrameBuffer::AddDepthAttachment(const Texture& texture) {
  Bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture.ID, 0);
}
