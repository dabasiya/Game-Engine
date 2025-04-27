#pragma once

#include <gladr.h>
#include "Texture.h"

struct FrameBuffer {
  unsigned int ID;

  FrameBuffer();
  void AddColorAttachment(unsigned int n, const Texture& texture);
  void AddDepthAttachment(const Texture& texture);
  void Bind();
  void Unbind();
  void Delete();
  void CheckError();
};
