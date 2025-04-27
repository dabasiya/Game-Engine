#include "UI.h"

#include <Window.h>
#include <Renderer2D.h>
#include <Development/Editorlayer.h>

#include <glm/gtc/matrix_transform.hpp>

FontRenderer* ui::fr;

std::stack<windowdata*> ui::windowstack;

std::vector<windowdata*> ui::windowdatas;

std::stack<uistyle> ui::stylestack;


inputid ui::activeinputbox;

bool ui::was_mouse_pressed = false;

int ui::panelsize = 30;
int ui::elementsize = 20;
int ui::fontsize = 14;

float ui::widthor = 0.0f;
float ui::heighto = 0.0f;
float ui::hor = 0.0f;
float ui::ho = 0.0f;


void ui::windowmove() {
	for (auto data : windowdatas) {

		if (data->visible) {

			if (mousehover(data->x, data->y, data->width - WINDOW_TAB_HEIGHT, WINDOW_TAB_HEIGHT))
			{
				if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !data->mouse_locked && !was_mouse_pressed) {
					if (data->z != 0.9f) {

						for (auto a : windowdatas) {
							a->z = 0.2f;
							a->mouse_locked = false;
						}
						data->z = 0.9f;
					}

					was_mouse_pressed = true;

					double x, y;
					glfwGetCursorPos(Window::ID, &x, &y);

					data->mouse_lock_x = (unsigned int)x;
					data->mouse_lock_y = (unsigned int)y;

					data->storex = data->x;
					data->storey = data->y;

					data->mouse_locked = true;

					break;
				}
			}

			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE && data->mouse_locked) {
				data->mouse_locked = false;

			}

			else if (data->mouse_locked) {
				double x, y;
				glfwGetCursorPos(Window::ID, &x, &y);

				data->x = data->storex + (x - data->mouse_lock_x);
				data->y = data->storey + (y - data->mouse_lock_y);
			}
		}

	}
}



bool ui::mousehover(int x, int y, int width, int height) {
	return ((Window::Window_Mousex > x && Window::Window_Mousex < x + width) && (Window::Window_Mousey > y && Window::Window_Mousey < y + height));
}


bool ui::onevent(Event& e) {

	if (e.is(Event_Type::Mouse_Pressed)) {
		for (auto data : windowdatas) {
			if (data->visible) {
				if (mousehover((data->x + data->width - WINDOW_CLOSE_BUTTON_SIZE), data->y + 5, WINDOW_CLOSE_BUTTON_SIZE, WINDOW_CLOSE_BUTTON_SIZE) && !was_mouse_pressed) {
					data->visible = false;
					was_mouse_pressed = true;
					return true;
				}
			}
		}
	}

	else if (e.is(Event_Type::Mouse_Scrolled)) {
		MouseScrolledEvent* me = (MouseScrolledEvent*)&e;
		for (auto data : windowdatas) {
			if (data->visible) {
				if (mousehover(data->x, data->y, data->width, data->maxheight)) {
					EditorLayer::camera_locked = true;
					if (me->yoffset < 0.0f) {
						data->offsety += (int)me->yoffset * 20;
					}
					else if (me->yoffset > 0.0f) {
						data->offsety += (int)me->yoffset * 20;
					}

					if (data->offsety > 0)
						data->offsety = 0;
				}
			}
		}
	}

	else if (e.is(Event_Type::Key_Typed)) {
		KeyTypedEvent* ke = (KeyTypedEvent*)&e;

		if (activeinputbox.type == inputtype::TEXT) {
			if ((ke->key >= 33 && ke->key <= 125) || ke->key == ' ') {
				activeinputbox.text += static_cast<char>(ke->key);
				return true;
			}
		}

		else if (activeinputbox.type == inputtype::FLOAT) {
			if (ke->key >= '0' && ke->key <= '9')
			{
				activeinputbox.text += static_cast<char>(ke->key);
				return true;
			}
			else if (ke->key == '-') {
				if (activeinputbox.text == "") {
					activeinputbox.text += "-";
					return true;
				}
			}
			else if (ke->key == '.') {
				if (activeinputbox.text.find(".") == std::string::npos) {
					activeinputbox.text += ".";
					return true;
				}
			}
		}

		else if (activeinputbox.type == inputtype::INT) {
			if (ke->key >= '0' && ke->key <= '9') {
				activeinputbox.text += static_cast<char>(ke->key);
				return true;
			}
		}
	}

	else if (e.is(Event_Type::Key_Pressed)) {
		KeyPressedEvent* ke = (KeyPressedEvent*)&e;

		if (ke->key == GLFW_KEY_BACKSPACE && ke->mod == GLFW_MOD_CONTROL) {
			if (activeinputbox.type == inputtype::FLOAT) {
				activeinputbox.text = "";
				return true;
			}
			else if (activeinputbox.type == inputtype::TEXT) {
				int index = activeinputbox.text.find_last_of(" ");
				if (index > 0) {
					activeinputbox.text = activeinputbox.text.substr(0, index);
					return true;
				}
				else {
					activeinputbox.text = "";
					return true;
				}
			}
		}

		else if (ke->key == GLFW_KEY_BACKSPACE && !activeinputbox.text.empty()) {
			activeinputbox.text.pop_back();
			return true;
		}
	}

	return false;
}


void ui::calculatevalues() {

	hor = (Window::OrthographicSize * Window::Ratio) / 2.0f;
	ho = Window::OrthographicSize / 2.0f;

	widthor = (1.0f / Window::Width) * Window::OrthographicSize * Window::Ratio;
	heighto = (1.0f / Window::Height) * Window::OrthographicSize;

}

void ui::Begin(const std::string& title, windowdata& data) {

	if (data.visible) {

		float ui_ortho = Window::OrthographicSize / 2.0f;
		glm::mat4 orthomatrix = glm::ortho(-Window::Ratio * ui_ortho, Window::Ratio * ui_ortho, -ui_ortho, ui_ortho);
		glEnable(GL_SCISSOR_TEST);
		glScissor(data.x, Window::Height - (data.y + 1000), data.width, 1000);
		float xpos = (float)data.x * widthor;
		xpos -= hor;
		float ypos = (float)data.y * heighto;
		ypos = -(ypos - ho);

		float width = (float)data.width * widthor;
		float height = (float)WINDOW_TAB_HEIGHT * heighto;

		float close_button_x = (float)(data.x + data.width - WINDOW_TAB_HEIGHT / 2) * widthor;
		close_button_x -= hor;

		float close_button_area_width = (float)WINDOW_TAB_HEIGHT * widthor;
		float close_button_area_height = (float)WINDOW_TAB_HEIGHT * heighto;

		float close_button_width = (float)WINDOW_CLOSE_BUTTON_SIZE * widthor;
		float close_button_height = (float)WINDOW_CLOSE_BUTTON_SIZE * heighto;

		Renderer2D::BeginScene(orthomatrix);

		Renderer2D::DrawQuad({ xpos, ypos, data.z + 0.1f }, { width, height }, data.tab_color, { 0.5f, -0.5f });
		fr->PrintStringui(title, xpos + (width - close_button_area_width) / 2, ypos - height / 2, data.z + 0.1f, 20.0f);

		float line_height = 5.0f * heighto;
		// line_width = close_button_width because both size is 25 pixel

		Renderer2D::DrawQuad({ close_button_x, (ypos - close_button_area_height / 2), data.z + 0.1f }, { close_button_width, close_button_height }, data.panel_color);
		Renderer2D::DrawRotatedQuad({ close_button_x, (ypos - close_button_area_height / 2), data.z + 0.1f }, { close_button_width, line_height }, 45.0f, data.tab_color);
		Renderer2D::DrawRotatedQuad({ close_button_x, (ypos - close_button_area_height / 2), data.z + 0.1f }, { close_button_width, line_height }, 135.0f, data.tab_color);

		data.content_offset_x = 0;
		data.content_offset_y = WINDOW_TAB_HEIGHT + data.offsety;

	}
	windowstack.push(&data);
}


void ui::End() {
	Renderer2D::EndScene();
	glDisable(GL_SCISSOR_TEST);
	auto& data = windowstack.top();
	windowstack.pop();
	data->maxheight = data->content_offset_y;
}

void ui::pushstyle(uistyle style) {
	stylestack.push(style);
}

void ui::popstyle() {
	stylestack.pop();
}


void ui::Panel(unsigned int aheight) {
	windowdata* data = windowstack.top();
	if (data->visible) {
		float xpos = (float)(data->x + data->content_offset_x) * widthor;
		xpos -= hor;
		float ypos = (float)(data->y + data->content_offset_y) * heighto;
		ypos = -(ypos - ho);

		float width = (float)data->width * widthor;
		float height = (float)aheight * heighto;

		Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, data->panel_color, { 0.5f, -0.5f });

		if (!stylestack.empty()) {
			uistyle style = stylestack.top();
			if (style == uistyle::row_two_block) {
				float xpos = (float)(data->x + data->width / 2) * widthor;
				xpos -= hor;
				float ypos = (float)(data->y + data->content_offset_y + aheight / 2) * heighto;
				ypos = -(ypos - ho);

				float width = 5.0f * widthor;
				float height = (float)aheight * heighto;

				Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, data->tab_color);
			}
		}

	}
}

void ui::updateuiparameters(windowdata* data, int& inputwidth, int& last_content_offsetx, int& last_content_offsety, float& width, float& height) {
	if (stylestack.empty() == false) {
		if (stylestack.top() == uistyle::row_two_block) {
			inputwidth = data->width / 2 - 12.5f;
			width = ((float)data->width / 2 - 12.5f) * widthor;
			if (data->content_offset_x != 0) {
				data->content_offset_x = 0;
				data->content_offset_y += panelsize;
			}
			else {
				Panel(panelsize);
				data->content_offset_x += (data->width / 2 + 2.5f);
			}
		}
	}
	else {
		if (data->content_offset_x != 0) {
			data->content_offset_x = 0;
			last_content_offsetx = 0;
			last_content_offsety += panelsize;
			data->content_offset_y += panelsize;
		}
		width = (float)(data->width - 10) * widthor;
		inputwidth = data->width - 10;
		Panel(panelsize);
		data->content_offset_y += panelsize;
	}

	height = (float)elementsize * heighto;
}

bool ui::Button(const std::string& text) {
	windowdata* data = windowstack.top();

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5) * widthor;
		xpos -= hor;
		float ypos = (float)(data->y + last_content_offsety + 5) * heighto;
		ypos = -(ypos - ho);

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, data->tab_color, { 0.5f, -0.5f });
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				activeinputbox.id = "";
				was_mouse_pressed = false;
				return true;
			}
		}
		else {
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, data->panel_color + glm::vec4(0.1f, 0.1f, 0.1f, 0.0f), { 0.5f, -0.5f });
		}
		fr->PrintStringui(text, xpos + width / 2, ypos - height / 2, data->z, (float)fontsize);
	}
	return false;
}

void ui::Separator() {

	windowdata* data = windowstack.top();

	if (data->visible) {

		if (data->content_offset_x != 0) {
			data->content_offset_x = 0;
			data->content_offset_y += panelsize;
		}

		float xpos = (float)(data->x) * widthor;
		xpos -= hor;
		float ypos = (float)(data->y + data->content_offset_y) * heighto;
		ypos = -(ypos - ho);

		float width = (float)data->width * widthor;
		float height = 5.0f * heighto;

		Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, data->tab_color, { 0.5f, -0.5f });

		data->content_offset_y += 5;
	}
}


void ui::InputBox(const std::string& id, std::string& text) {
	windowdata* data = windowstack.top();

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5) * widthor;
		xpos -= hor;
		float ypos = (float)(data->y + last_content_offsety + 5) * heighto;
		ypos = -(ypos - ho);


		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				activeinputbox = { id, inputtype::TEXT, text };
				was_mouse_pressed = true;
				EditorLayer::camera_locked = true;
			}
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, glm::vec4(1.0f), { 0.5f, -0.5f });
		}
		else if (activeinputbox.id == id)
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, glm::vec4(1.0f), { 0.5f, -0.5f });
		else
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, glm::vec4(0.8f), { 0.5f, -0.5f });

		if (activeinputbox.id == id) {
			text = activeinputbox.text;
		}

		fr->PrintStringui(text, xpos + width / 2, ypos - height / 2, data->z, (float)fontsize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}

}


void ui::StateButton(const std::string& text, bool& value, bool value2) {
	windowdata* data = windowstack.top();

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5) * widthor;
		xpos -= hor;
		float ypos = (float)(data->y + last_content_offsety + 5) * heighto;
		ypos = -(ypos - ho);

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				value = value2;
				was_mouse_pressed = true;
				activeinputbox.id = "";
			}
		}

		if (value == value2) {
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, data->tab_color, { 0.5f, -0.5f });
		}
		else {
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, data->panel_color + glm::vec4(0.1f, 0.1f, 0.1f, 0.0f), { 0.5f, -0.5f });
		}
		fr->PrintStringui(text, xpos + width / 2, ypos - height / 2, data->z, (float)fontsize);
	}
}

void ui::Label(const std::string& text) {
	windowdata* data = windowstack.top();

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5) * widthor;
		xpos -= hor;
		float ypos = (float)(data->y + last_content_offsety + 5) * heighto;
		ypos = -(ypos - ho);

		fr->PrintStringui(text, xpos + width / 2, ypos - height / 2, data->z, (float)fontsize);

	}


}


void ui::FloatInputBox(const std::string& id, float& value, float onclickvalue) {
	windowdata* data = windowstack.top();

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5) * widthor;
		xpos -= hor;
		float ypos = (float)(data->y + last_content_offsety + 5) * heighto;
		ypos = -(ypos - ho);

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				activeinputbox = { id, inputtype::FLOAT, std::to_string(value) };
				was_mouse_pressed = true;
				EditorLayer::camera_locked = true;
			}

			else if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_2) && !was_mouse_pressed) {
				value = onclickvalue;
				was_mouse_pressed = true;
				activeinputbox = { id, inputtype::FLOAT, std::to_string(value) };
				activeinputbox.text = std::to_string(value);
				EditorLayer::camera_locked = true;
			}

			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, glm::vec4(1.0f), { 0.5f, -0.5f });
		}

		else if (activeinputbox.id == id)
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, glm::vec4(1.0f), { 0.5f, -0.5f });
		else
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, glm::vec4(0.8f), { 0.5f, -0.5f });


		if (activeinputbox.id == id)
			fr->PrintStringui(activeinputbox.text, xpos + width / 2, ypos - height / 2, data->z, (float)fontsize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		else
			fr->PrintStringui(std::to_string(value), xpos + width / 2, ypos - height / 2, data->z, (float)fontsize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));


		if (activeinputbox.id == id) {
			value = atof(activeinputbox.text.c_str());
		}
	}

}



// check box

bool ui::CheckBox(const std::string& text, bool& value) {
	windowdata* data = windowstack.top();

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 7) * widthor;
		xpos -= hor;
		float ypos = (float)(data->y + last_content_offsety + 7) * heighto;
		ypos = -(ypos - ho);

		Renderer2D::DrawQuad({ xpos, ypos, data->z }, { height, height }, glm::vec4(1.0f), { 0.5f, -0.5f });
		fr->PrintStringui(text, xpos + width / 2, ypos - height / 2, data->z, (float)fontsize);

		if (value) {
			Renderer2D::DrawQuad({ xpos + (height / 2), ypos - height / 2, data->z }, { (height * 0.7f), height * 0.7f }, { 0.0f, 0.0f, 0.0f, 1.0f });
		}

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, 30, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				was_mouse_pressed = true;
				value = !value;
				return value;
			}
		}
	}
	return value;
}


void ui::OptionSelector(std::vector<std::string> options, unsigned int& selectedoptionindex) {
	windowdata* data = windowstack.top();

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5) * widthor;
		xpos -= hor;
		float ypos = (float)(data->y + last_content_offsety + 5) * heighto;
		ypos = -(ypos - ho);

		float xpos2 = (float)(data->x + inputwidth + last_content_offsetx) * widthor;
		xpos2 -= hor;

		float textoffset = (float)(inputwidth / 2) * widthor;

		Renderer2D::DrawQuad({ xpos, ypos, data->z }, { height, height }, data->tab_color, { 0.5f, -0.5f });
		float trianglesize = height * 0.7f;

		Renderer2D::DrawQuad({ xpos2, ypos, data->z }, { height, height }, data->tab_color, { -0.5f, -0.5f });

		fr->PrintStringui(options[selectedoptionindex], xpos + textoffset, ypos - height / 2, data->z, (float)fontsize);

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, 30, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				was_mouse_pressed = true;
				if (selectedoptionindex > 0)
					selectedoptionindex--;
			}
			Renderer2D::DrawRotatedTriangle({ xpos + height / 2, ypos - height / 2, data->z }, { trianglesize, trianglesize }, 90.0f, glm::vec4(1.0f));
		}

		else
			Renderer2D::DrawRotatedTriangle({ xpos + height / 2, ypos - height / 2, data->z }, { trianglesize, trianglesize }, 90.0f, data->panel_color);


		if (mousehover(data->x + inputwidth + last_content_offsetx - 30, data->y + last_content_offsety + 5, 30, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				was_mouse_pressed = true;
				if (selectedoptionindex < options.size() - 1)
					selectedoptionindex++;
			}
			Renderer2D::DrawRotatedTriangle({ xpos2 - height / 2, ypos - height / 2, data->z }, { trianglesize, trianglesize }, -90.0f, glm::vec4(1.0f));
		}
		else
			Renderer2D::DrawRotatedTriangle({ xpos2 - height / 2, ypos - height / 2, data->z }, { trianglesize, trianglesize }, -90.0f, data->panel_color);
	}
}


void ui::Image(SubTexture* image) {
	windowdata* data = windowstack.top();

	if (data->visible) {
		if (data->content_offset_x != 0) {
			Panel(panelsize);
		}

		float cwidth = image->coords2.x - image->coords1.x;
		float cheight = image->coords1.y - image->coords2.y;
		unsigned int index = image->index;


		Texture* s_texture = Renderer2D::textures[index];

		if (s_texture) {
			float width = 0.0f, height = 0.0f;
			if (cwidth * s_texture->width > cheight * s_texture->height) {
				width = 1.0f;
				height = (cheight * s_texture->height) / (cwidth * s_texture->width);
			}
			else {
				width = (cwidth * s_texture->width) / (cheight * s_texture->height);
				height = 1.0f;
			}



			float x = (float)(data->x + data->content_offset_x) * widthor;
			x -= hor;
			float y = (float)(data->y + data->content_offset_y) * heighto;
			y = -(y - ho);
			float yoffset = 200.0f * heighto;
			float xoffset = 200.0f * widthor;

			float wr = 400.0f * widthor;
			float hr = 400.0f * heighto;

			Renderer2D::DrawQuad({ x, y, data->z }, { ((float)400 / Window::Width) * Window::OrthographicSize * Window::Ratio, ((float)400 / Window::Height) * Window::OrthographicSize }, data->panel_color, { 0.5f, -0.5f });
			Renderer2D::DrawQuad({ x + xoffset, y - yoffset, data->z }, { width * wr, height * hr }, *image);


			data->content_offset_y += 400;
		}
	}
}


void ui::UIntInputBox(const std::string& id, unsigned int& value, int onclickvalue) {
	windowdata* data = windowstack.top();

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5) * widthor;
		xpos -= hor;
		float ypos = (float)(data->y + last_content_offsety + 5) * heighto;
		ypos = -(ypos - ho);

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				was_mouse_pressed = true;
				activeinputbox = { id, inputtype::INT, std::to_string(value) };
				EditorLayer::camera_locked = true;
			}

			else if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_2) && !was_mouse_pressed) {
				was_mouse_pressed = true;
				value = onclickvalue;
				activeinputbox = { id, inputtype::INT, std::to_string(value) };
				EditorLayer::camera_locked = true;
			}
		}



		// if input box selected its highlighted
		if (activeinputbox.id == id)
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, glm::vec4(1.0f), { 0.5f, -0.5f });
		else
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, glm::vec4(0.8f), { 0.5f, -0.5f });

		
		if (activeinputbox.id == id)
			fr->PrintStringui(activeinputbox.text, xpos + width / 2, ypos - height / 2, data->z, (float)fontsize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		else
			fr->PrintStringui(std::to_string(value), xpos + width / 2, ypos - height / 2, data->z, (float)fontsize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		

		if (activeinputbox.id == id) {
			value = atof(activeinputbox.text.c_str());
		}
	}

}


// for uint16


void ui::UInt16InputBox(const std::string& id, uint16_t& value, int onclickvalue) {
	windowdata* data = windowstack.top();

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5) * widthor;
		xpos -= hor;
		float ypos = (float)(data->y + last_content_offsety + 5) * heighto;
		ypos = -(ypos - ho);

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				activeinputbox = { id, inputtype::INT, std::to_string(value) };
				EditorLayer::camera_locked = true;
				was_mouse_pressed = true;
			}

			else if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_2) && !was_mouse_pressed) {
				value = onclickvalue;
				activeinputbox = { id, inputtype::INT, std::to_string(value) };
				was_mouse_pressed = true;
			}
		}



		// if input box selected its highlighted
		if (activeinputbox.id == id)
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, glm::vec4(1.0f), { 0.5f, -0.5f });
		else
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, glm::vec4(0.8f), { 0.5f, -0.5f });

		
		if (activeinputbox.id == id)
			fr->PrintStringui(activeinputbox.text, xpos + width / 2, ypos - height / 2, data->z, (float)fontsize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		else
			fr->PrintStringui(std::to_string(value), xpos + width / 2, ypos - height / 2, data->z, (float)fontsize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		
		if (activeinputbox.id == id) {
			value = atof(activeinputbox.text.c_str());
		}
	}

}


bool ui::DropDownButton(const std::string& text, bool value) {
	windowdata* data = windowstack.top();

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;


		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5) * widthor;
		xpos -= hor;
		float ypos = (float)(data->y + last_content_offsety + 5) * heighto;
		ypos = -(ypos - ho);

		float buttonwidth = 15.0f * widthor;
		float buttonheight = 15.0f * heighto;

		Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, height }, data->panel_color + glm::vec4(.1f, .1f, .1f, .0f), { .5f, -.5f });

		Renderer2D::DrawRotatedTriangle({ xpos + height / 1.7f, ypos - height / 2, data->z }, { buttonwidth, buttonheight }, -90.0f, data->tab_color);

		fr->PrintStringui(text, xpos + width / 2, ypos - height / 2, data->z, (float)fontsize);
	}
	return false;
}
