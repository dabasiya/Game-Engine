#include "windowsinput.h"

bool Input::KeyPressed(unsigned int value) {
	return glfwGetKey(Window::ID, value);
}
