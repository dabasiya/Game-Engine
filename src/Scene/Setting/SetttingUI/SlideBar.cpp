#include "SlideBar.h"
#include <ui/UI.h>
#include <Window.h>

#include <scene/Setting/Setting.h>

SlideBar::SlideBar()
{

}

SlideBar::SlideBar(unsigned int a_min, unsigned int a_max)
	: value(0), min(a_min), max(a_max)
{

}

SlideBar::SlideBar(unsigned int a_value, unsigned int a_min, unsigned int a_max)
	: value(a_value), min(a_min), max(a_max)
{

}

void SlideBar::Render() {



	std::string output = text + " : " + std::to_string(value);

	ui::fr->PrintString(output, x - Window::Ratio * 0.5f, y, 80.0f);

	height = (30.0f / Window::Height);
	width = (800.0f / Window::Width) * Window::Ratio;

	float spx = x + Window::Ratio * 0.5f;
	float spy = y;
	Renderer2D::DrawQuad({ spx, y }, { width, height }, glm::vec4(1.0f));

	float mousex = Window::Mousex / 2.0f;
	float mousey = Window::Mousey / 2.0f;


	if ((mousex >= spx - (width / 2.0f) && mousex <= spx + (width / 2.0f)) &&
		(mousey <= spy + height / 2.0f && mousey >= spy - height / 2.0f) &&
		!Setting::Mouse_Pressed &&
		glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1)) {
		unsigned int range = max - min;

		value = ((mousex - (spx - (width / 2.0f))) / width) * range;
	}

	float rp = spx - (width / 2.0f) + ((float)value / (max - min)) * width;
	Renderer2D::DrawQuad({ rp, spy }, { 0.05f, 0.05f }, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
}