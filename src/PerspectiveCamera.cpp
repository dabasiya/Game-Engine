#include "PerspectiveCamera.h"
#include <iostream>

void PerspectiveCamera::Update() {
	float ratio = (float)Window::Width / Window::Height;
	viewprojection = glm::perspective(glm::radians(angle), ratio, near, far);
}
