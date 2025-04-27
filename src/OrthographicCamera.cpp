#include <OrthographicCamera.h>
#include <glm/gtc/matrix_transform.hpp>


OrthographicCamera::OrthographicCamera(float size, float a_near, float a_far)
	: orthographicsize(size), near(a_near), far(a_far)
{
	viewprojection = glm::ortho(-Window::Ratio * (size / 2), Window::Ratio * (size / 2), -(size / 2), size / 2, a_near, a_far);
}


void OrthographicCamera::Update() {

	viewprojection = glm::ortho(-Window::Ratio * (orthographicsize / 2), Window::Ratio * (orthographicsize / 2), -(orthographicsize / 2), orthographicsize / 2, near, far);
}
