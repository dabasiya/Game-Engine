#include "PerspectiveCamera.h"
#include <iostream>

void PerspectiveCamera::Update() {
	if (Window::Width != 0 && Window::Height != 0) {
		float ratio = (float)Window::Width / Window::Height;
		viewprojection = glm::perspective(glm::radians(angle), ratio, near, far);
	}
}
