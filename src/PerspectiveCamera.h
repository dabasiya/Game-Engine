#pragma once

#include <window.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


struct PerspectiveCamera {

	float angle = 45.0f;

	float near = 0.1f;
	float far = 100.0f;

	glm::mat4 viewprojection;

	PerspectiveCamera() {
		Update();
	}

	void Update();
};
