#pragma once

#include <glm/glm.hpp>

struct SubTexture {

	// for image index
	unsigned int index = 0;

	// subtexture coordinates

	glm::vec2 coords1 = glm::vec2(0.0f), coords2 = glm::vec2(0.0f);


	// this is for rendering
	// any animated object have different size of sprite in their animation and if we use same size for 
	// rendering animation then object look weird so we use x and y multiplier in a subtexture so object 
	// rendering with its original size multiplied by this multiplier
	float X_Multiplier = 1.0f;
	float Y_Multiplier = 1.0f;


	// constructor

	SubTexture() {}

	SubTexture(unsigned int a_index, const glm::vec2& a_coords1, const glm::vec2& a_coords2)
		: index(a_index), coords1(a_coords1), coords2(a_coords2)
	{

	}

	SubTexture(unsigned int a_index, const glm::vec2& a_coords1, const glm::vec2& a_coords2, float width, float height)
		: index(a_index), coords1(a_coords1), coords2(a_coords2), X_Multiplier(width), Y_Multiplier(height)
	{}

};
