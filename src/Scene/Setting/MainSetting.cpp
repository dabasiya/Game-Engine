#include "MainSetting.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Window.h>

#include <ui/UI.h>

#include "Setting.h"

void MainSetting::Render() {

	glm::mat4 ortho = glm::ortho(-Window::Ratio, Window::Ratio, -1.0f, 1.0f);
	Renderer2D::BeginScene(ortho);

	Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 2.0f, 2.0f }, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	ui::fr->PrintString("Setting", 0.0f, 0.7f, 256.0f);

	test_sb.Render();

	Renderer2D::EndScene();

	if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1))
		Setting::Mouse_Pressed = true;
	else
		Setting::Mouse_Pressed = false;
}