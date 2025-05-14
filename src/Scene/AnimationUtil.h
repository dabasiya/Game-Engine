#pragma once

#include <animation.h>
#include <string>

Animation readanimation(const std::string& filepath);

void writeanimation(const std::string& filepath, Animation m_animation);

