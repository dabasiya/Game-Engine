#pragma once

#include <window.h>
#include <glm/glm.hpp>



struct OrthographicCamera {

	float near = -1.0f;
	float far = 1.0f;

	float orthographicsize = 0.0f;


	glm::mat4 viewprojection;

	OrthographicCamera() {}
	OrthographicCamera(float size, float a_near, float a_far);

	void Update();
};
