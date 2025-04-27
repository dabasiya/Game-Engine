#pragma once

#include <SubTexture.h>
#include <vector>


struct Animation {

	float time = 0.0f;

	std::vector<SubTexture> frames;
	std::vector<float> frametimes;

	void Append(SubTexture s1, float a_time);

	void Update(float ts);

	SubTexture getframe();
};
