#pragma once


#include <Renderer2D.h>

struct SlideBar {

	unsigned int value = 0;

	unsigned int min = 0;
	unsigned int max = 100;

	float x = 0.0f, y = 0.0f;

	float width = 0.0f, height = 0.0f;


	SlideBar();
	SlideBar(unsigned int a_min, unsigned int a_max);
	SlideBar(unsigned int a_value, unsigned int a_min, unsigned int a_max);

	std::string text = "Volume";

	void Render();
};