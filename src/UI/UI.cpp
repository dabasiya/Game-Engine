#include "UI.h"

#include <Window.h>
#include <Renderer2D.h>
#include <Development/Editorlayer.h>

#include <glm/gtc/matrix_transform.hpp>

#include <Application.h>

#include <Instrumentor.h>

std::shared_ptr<FontRenderer> ui::fr;

std::stack<windowdata*> ui::windowstack;

std::vector<windowdata*> ui::windowdatas;

std::stack<uistyle> ui::stylestack;

windowdata* ui::activewindow;

uint32_t ui::activeelementid;
std::unordered_map<uint32_t, InputState> ui::uiInputStates;

bool ui::was_mouse_pressed = false;

int ui::panelsize = 30;
int ui::elementsize = 20;
int ui::fontsize = 14;

double ui::widthor = 0.0;
double ui::heighto = 0.0;
double ui::hor = 0.0;
double ui::ho = 0.0;

std::vector<glm::vec4> cpcolors(4);
std::vector<std::vector<glm::vec4>> cpmaincolors;
std::vector<glm::vec4> opacitycolors(4);

glm::vec4* ui::selectedcolor;

std::vector<int> ui::quadindexes;
std::vector<glm::vec4> ui::cliprects;


SubTexture checkboxboundary = { 13, {0.0f, 0.75f}, {0.125f, 0.625f} };
SubTexture checkboxcenter = { 13, {0.125f, 0.75f}, {0.25f, 0.625f} };


glm::vec4 getcolorfromdegree(float degree) {
	if (degree <= 60.0f) {
		float g = degree / 60.0f;
		return glm::vec4(1.0f, g, 0.0f, 1.0f);
	}
	else if (degree <= 120.0f) {
		float d = degree - 60.0f;
		float r = 1.0f - (d / 60.0f);
		return glm::vec4(r, 1.0f, 0.0f, 1.0f);
	}
	else if (degree <= 180.0f) {
		float d = degree - 120.0f;
		float b = d / 60.0f;
		return glm::vec4(0.0f, 1.0f, b, 1.0f);
	}
	else if (degree <= 240.0f) {
		float d = degree - 180.0f;
		float g = 1.0f - (d / 60.0f);
		return glm::vec4(0.0f, g, 1.0f, 1.0f);
	}
	else if (degree <= 300.0f) {
		float d = degree - 240.0f;
		float r = d / 60.0f;
		return glm::vec4(r, 0.0f, 1.0f, 1.0f);
	}
	else if (degree <= 360.0f) {
		float d = degree - 300.0f;
		float b = 1.0f - (d / 60.0f);
		return glm::vec4(1.0f, 0.0f, b, 1.0f);
	}
	return glm::vec4(1.0f);
}

void ui::windowmove() {
	
}



bool ui::mousehover(int x, int y, int width, int height) {
    int imx = (int)Window::Window_Mousex;
	int imy = (int)Window::Window_Mousey;
    return ((imx > x && imx < x + width) && (imy > y && imy < y + height));
}


bool ui::onevent(Event& e) {

	InputState& state = uiInputStates[activeelementid];

	
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

		if (state.type == InputType::TEXT) {
			if ((ke->key >= 33 && ke->key <= 125) || ke->key == ' ') {
				state.text += static_cast<char>(ke->key);
				return true;
			}
		}

		else if (state.type == InputType::FLOAT) {
			if (ke->key >= '0' && ke->key <= '9')
			{
				state.text += static_cast<char>(ke->key);
				return true;
			}
			else if (ke->key == '-') {
				if (state.text == "") {
					state.text += "-";
					return true;
				}
			}
			else if (ke->key == '.') {
				if (state.text.find(".") == std::string::npos) {
					state.text += ".";
					return true;
				}
			}
		}

		else if (state.type == InputType::INT) {
			if (ke->key >= '0' && ke->key <= '9') {
				state.text += static_cast<char>(ke->key);
				return true;
			}
		}
	}

	else if (e.is(Event_Type::Key_Pressed)) {
		KeyPressedEvent* ke = (KeyPressedEvent*)&e;

		if (ke->key == GLFW_KEY_BACKSPACE && ke->mod == GLFW_MOD_CONTROL) {
			if (state.type == InputType::FLOAT) {
				state.text = "";
				return true;
			}
			else if (state.type == InputType::TEXT) {
				int index = state.text.find_last_of(" ");
				if (index > 0) {
					state.text = state.text.substr(0, index);
					return true;
				}
				else {
					state.text = "";
					return true;
				}
			}
		}

		else if (ke->key == GLFW_KEY_BACKSPACE && !state.text.empty()) {
			state.text.pop_back();
			return true;
		}
	}

	return false;
}

void ui::resetInputs() {
	for (auto& pair : uiInputStates) {
		pair.second.text.clear();
	}
}

void ui::Init() {

  // init cp colors for color selection

  cpcolors[0] = {0.0f, 0.0f, 0.0f, 1.0f};
  cpcolors[1] = {1.0f, 1.0f, 1.0f, 1.0f};
  cpcolors[2] = { 0.0f, 0.0f, 0.0f, 1.0f };
  cpcolors[3] = { 0.0f, 0.0f, 0.0f, 1.0f };

  opacitycolors[0] = { 1.0f, 1.0f, 1.0f, 1.0f };
  opacitycolors[1] = { 1.0f, 1.0f, 1.0f, 0.0f };
  opacitycolors[2] = { 1.0f, 1.0f, 1.0f, 0.0f };
  opacitycolors[3] = { 1.0f, 1.0f, 1.0f, 1.0f };

  std::vector<glm::vec4> m1(4);
  std::vector<glm::vec4> m2(4);
  std::vector<glm::vec4> m3(4);
  std::vector<glm::vec4> m4(4);
  std::vector<glm::vec4> m5(4);
  std::vector<glm::vec4> m6(4);

  m1 = {
	  {1.0f, 1.0f, 0.0f, 1.0f},
	  {1.0f, 0.0f, 0.0f, 1.0f},
	  {1.0f, 0.0f, 0.0f, 1.0f},
	  {1.0f, 1.0f, 0.0f, 1.0f}
  };

  m2 = {
	  {0.0f, 1.0f, 0.0f, 1.0f},
	  {1.0f, 1.0f, 0.0f, 1.0f},
	  {1.0f, 1.0f, 0.0f, 1.0f},
	  {0.0f, 1.0f, 0.0f, 1.0f}
  };
  m3 = {
	  {0.0f, 1.0f, 1.0f, 1.0f},
	  {0.0f, 1.0f, 0.0f, 1.0f},
	  {0.0f, 1.0f, 0.0f, 1.0f},
	  {0.0f, 1.0f, 1.0f, 1.0f}
  };
  m4 = {
	  {0.0f, 0.0f, 1.0f, 1.0f},
	  {0.0f, 1.0f, 1.0f, 1.0f},
	  {0.0f, 1.0f, 1.0f, 1.0f},
	  {0.0f, 0.0f, 1.0f, 1.0f}
  };
  m5 = {
	  {1.0f, 0.0f, 1.0f, 1.0f},
	  {0.0f, 0.0f, 1.0f, 1.0f},
	  {0.0f, 0.0f, 1.0f, 1.0f},
	  {1.0f, 0.0f, 1.0f, 1.0f}
  };
  m6 = {
	  {1.0f, 0.0f, 0.0f, 1.0f},
	  {1.0f, 0.0f, 1.0f, 1.0f},
	  {1.0f, 0.0f, 1.0f, 1.0f},
	  {1.0f, 0.0f, 0.0f, 1.0f}
  };

  cpmaincolors.push_back(m1);
  cpmaincolors.push_back(m2);
  cpmaincolors.push_back(m3);
  cpmaincolors.push_back(m4);
  cpmaincolors.push_back(m5);
  cpmaincolors.push_back(m6);
  

}


void ui::calculatevalues() {

	hor = (Window::OrthographicSize * Window::Ratio) / 2.0f;
	ho = Window::OrthographicSize / 2.0f;

	widthor = (1.0f / Window::Width) * Window::OrthographicSize * Window::Ratio;
	heighto = (1.0f / Window::Height) * Window::OrthographicSize;

}

void ui::Begin(const char* title, windowdata& data) {

	Renderer2D::SetTexture(fr->fonttexture, fr->textureindex);

	if (data.visible) {

		unsigned int id = Renderer2D::state.indicespointer / 6;
		glm::vec4 clip = glm::vec4(data.x, data.y, data.x + data.width, Window::Height);

		quadindexes.push_back(id);
		cliprects.push_back(clip);

		// manage movement
		{
			if (mousehover(data.x, data.y, data.width - WINDOW_TAB_HEIGHT, WINDOW_TAB_HEIGHT))
			{
				if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !data.mouse_locked && !was_mouse_pressed) {
					if (data.z != 0.9f) {

						for (auto a : windowdatas) {
							a->z = 0.2f;
							a->mouse_locked = false;
						}
						data.z = 0.9f;
					}

					was_mouse_pressed = true;


					data.mouse_lock_x = (unsigned int)Window::Window_Mousex;
					data.mouse_lock_y = (unsigned int)Window::Window_Mousey;

					data.storex = data.x;
					data.storey = data.y;

					data.mouse_locked = true;
				}
			}

			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE && data.mouse_locked) {
				data.mouse_locked = false;
			}

			else if (data.mouse_locked) {
				double curx, cury;
				glfwGetCursorPos(Window::ID, &curx, &cury);

				

				int dx = (int)std::round(data.mouse_lock_x - (double)data.storex);
				int dy = (int)std::round(data.mouse_lock_y - (double)data.storey);
				data.x = curx - dx;
				data.y = cury - dy;
			}
		}

		Renderer2D::DrawQuad({ data.x, data.y, data.z + 0.1f }, { data.width, WINDOW_TAB_HEIGHT }, data.tab_color, { 0.5f, 0.5f });
		fr->PrintStringui(title, data.x + (data.width - WINDOW_TAB_HEIGHT) / 2, data.y + WINDOW_TAB_HEIGHT / 2, data.z + 0.1f, 20.0f);

		float close_button_x = data.x + data.width - WINDOW_TAB_HEIGHT / 2;

		float ypos = (float)data.y + (float)WINDOW_TAB_HEIGHT / 2.0f;
		Renderer2D::DrawQuad({ close_button_x, ypos, data.z + 0.1f }, { WINDOW_TAB_HEIGHT-3, WINDOW_TAB_HEIGHT-3 }, data.panel_color);
		Renderer2D::DrawRotatedQuad({ close_button_x, ypos, data.z + 0.1f }, { WINDOW_TAB_HEIGHT, 5.0f }, 45.0f, data.tab_color);
		Renderer2D::DrawRotatedQuad({ close_button_x, ypos, data.z + 0.1f }, { WINDOW_TAB_HEIGHT, 5.0f }, 135.0f, data.tab_color);

		data.content_offset_x = 0;
		data.content_offset_y = WINDOW_TAB_HEIGHT + data.offsety;


	}

	activewindow = &data;
}


void ui::End() {
	activewindow->maxheight = activewindow->content_offset_y;
	activewindow = nullptr;
}

void ui::pushstyle(uistyle style) {
	stylestack.push(style);
}

void ui::popstyle() {
	stylestack.pop();
}


void ui::Panel(unsigned int aheight) {
	windowdata* data = activewindow;
	if (data->visible) {
		float width = (float)data->width;

		Renderer2D::DrawQuad({ data->x + data->content_offset_x, data->y + data->content_offset_y, data->z }, { data->width, aheight }, data->panel_color, { 0.5f, 0.5f });

		if (!stylestack.empty()) {
			uistyle style = stylestack.top();
			if (style == uistyle::row_two_block) {
				float x = (float)(data->x + data->width / 2);
				float y = (float)(data->y + data->content_offset_y + aheight / 2);

				Renderer2D::DrawQuad({ x, y, data->z }, { 5.0f, aheight }, data->tab_color);
			}
		}

	}
}


// gives element width, by checking row_two_block is enabled or not
void ui::updateuiparameters(windowdata* data, int& inputwidth, int& last_content_offsetx, int& last_content_offsety, float& width, float& height) {
	
	if (stylestack.empty() == false) {
		if (stylestack.top() == uistyle::row_two_block) {
			inputwidth = data->width / 2 - 12.5f;
			width = ((float)data->width / 2 - 12.5f);
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
		width = (float)(data->width - 10);
		inputwidth = data->width - 10;
		Panel(panelsize);
		data->content_offset_y += panelsize;
	}

	height = (float)elementsize;
}

bool ui::Button(const char* text, MouseButtonClick c) {

	windowdata* data = activewindow;

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float x = (float)(data->x + last_content_offsetx + 5);
		if (mousehover(x, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			Renderer2D::DrawQuad({ x, data->y + last_content_offsety + 5, data->z }, { width, height }, data->tab_color, { 0.5f, 0.5f });
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT) && !was_mouse_pressed && c == MouseButtonClick::LEFT_CLICK) {
				activeelementid = 0;
				was_mouse_pressed = true;
				return true;
			}
			else if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_RIGHT) && !was_mouse_pressed) {
				return true;
			}
		}
		else {
			Renderer2D::DrawQuad({ x, data->y + last_content_offsety + 5, data->z }, { width, height }, data->panel_color + glm::vec4(0.1f, 0.1f, 0.1f, 0.0f), { 0.5f, 0.5f });
		}
		fr->PrintStringui(text, x +  width / 2, data->y + last_content_offsety + 5 + (height / 2), data->z, (float)fontsize);
	}
	return false;
}

void ui::ColorEdit(glm::vec4& color) {
	windowdata* data = activewindow;

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float x = data->x + last_content_offsetx + 5;
		float y = data->y + last_content_offsety + 5;

		Renderer2D::DrawQuad({ x, y, data->z }, { width, -height }, color, { 0.5f, -0.5f });

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT) && !was_mouse_pressed) {
				selectedcolor = &color;
				Application::GetInstance().m_editorlayer->colorselectorwindowdata.visible = true;
			}
		}
	}
}


void ui::ColorPicker() {
  windowdata* data = activewindow;

	if (data->visible) {

		float degree = 0.0f;
		int max = 0;
		int min = 0;
		float maxval = 0.0f;
		float minval = 0.0f;
		// find degree of main color 
		{
			if (selectedcolor->x <= selectedcolor->y && selectedcolor->x <= selectedcolor->z) {
				min = 0;
				minval = selectedcolor->x;
			}
			else if (selectedcolor->y <= selectedcolor->z) {
				min = 1;
				minval = selectedcolor->y;
			}
			else {
				min = 2;
				minval = selectedcolor->z;
			}

			if (selectedcolor->x >= selectedcolor->y && selectedcolor->x >= selectedcolor->z) {
				max = 0;
				maxval = selectedcolor->x;
			}
			else if (selectedcolor->y >= selectedcolor->z) {
				max = 1;
				maxval = selectedcolor->y;
			}
			else {
				max = 2;
				maxval = selectedcolor->z;
			}

			// degree
			if (max == 0) {
				degree = 60.0f * (selectedcolor->y - selectedcolor->z) / (maxval - minval);
			}
			else if (max == 1) {
				degree = 60.0f * (2 + ((selectedcolor->z - selectedcolor->x) / (maxval - minval)));
			}
			else {
				degree = 60.0f * (4 + ((selectedcolor->x - selectedcolor->y) / (maxval - minval)));
			}

			if (maxval - minval == 0.0f)
				degree = 0.0f;

			if (degree <= 0.0f)
				degree += 360.0f;
		}

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

        float colorsatwidth = (float)(data->width - 10 - 100);
		Panel(data->width - panelsize - 85);
		data->content_offset_y += data->width - panelsize - 85;
		float heightsector = colorsatwidth / 6.0f;

		float trianglewidth = 10.0f;

		float opacitytriangle = selectedcolor->a;

		cpcolors[2] = getcolorfromdegree(degree);

		float v = maxval;
		float s = (maxval - minval) / maxval;

		float sx = s;
		float sy = 1.0f - v;
		sx *= colorsatwidth;
		sy *= colorsatwidth;

		float nsx = data->x + last_content_offsetx + 5;
		float nsy = data->y + last_content_offsety + 5;

		// saturation
        Renderer2D::DrawQuad({nsx, nsy, data->z}, {colorsatwidth, -colorsatwidth}, cpcolors, {0.5f, -0.5f});
		Renderer2D::DrawQuad({ nsx + sx, nsy + sy, data->z }, { trianglewidth, trianglewidth }, EditorLayer::EditorLayerIcons["ColorSelect"]);

		Renderer2D::DrawQuad({ nsx + colorsatwidth + 2, nsy-3, data->z }, { 26, -(colorsatwidth+6) }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.5f, -0.5f });
		
		// render main color pallate
		for (unsigned int i = 0; i < 6; i++) {
			Renderer2D::DrawQuad({ nsx + colorsatwidth + 5, nsy + (heightsector * i), data->z}, {20.0f, -heightsector}, cpmaincolors[i], glm::vec2(0.5f, -0.5f));
		}

		// draw triangle at main colors
		float nv = degree / 360.0f;
		Renderer2D::DrawRotatedTriangle({ nsx + colorsatwidth + 25, nsy + (nv * colorsatwidth), data->z }, { trianglewidth, trianglewidth }, 90.0f, glm::vec4(1.0f));

		int x = data->x + last_content_offsetx + 5 + (data->width - 5 - 100);
		int y = data->y + last_content_offsety + 5;

		int mx = data->x + last_content_offsetx + 5;
		int my = data->y + last_content_offsety + 5;


		if (mousehover(x, y, 20, data->width - 110)) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT)) {
				int my = Window::Window_Mousey;
				int ny = my - y;
				float nd = (float)ny / (float)(data->width - 110);
				glm::vec4 newcolor = getcolorfromdegree(360.0f * nd);
				*selectedcolor = { newcolor.x, newcolor.y, newcolor.z, selectedcolor->a };
			}
		}
		else if (mousehover(x + 30, y, 20, data->width - 110)) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT)) {
				int my = Window::Window_Mousey;
				int ny = my - y;
				float nd = (float)ny / (float)(data->width - 110);
				selectedcolor->a = nd;
			}
		}
		else if (mousehover(mx, my, data->width - 110, data->width - 110)) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT)) {
				int sx = Window::Window_Mousex;
				int sy = Window::Window_Mousey;
				float nsx = (float)(Window::Window_Mousex - mx) / (float)(data->width - 110);
				float nsy = (float)(Window::Window_Mousey - my) / (float)(data->width - 110);
				glm::vec4 color = getcolorfromdegree(degree);
				glm::vec3 c1 = glm::mix(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(color), nsx);
				glm::vec3 c2 = glm::mix(c1, glm::vec3(0.0f, 0.0f, 0.0f), nsy);
				*selectedcolor = { c2.x, c2.y, c2.z, selectedcolor->a };
			}
		}

		// render opacity plage
		Renderer2D::DrawQuad({ x+31, y, data->z }, { 20, -colorsatwidth }, opacitycolors, { 0.5f, -0.5f });
		Renderer2D::DrawRotatedTriangle({ x+51, y + opacitytriangle*colorsatwidth, data->z }, { trianglewidth, trianglewidth }, 90.0f, glm::vec4(1.0f));
	}
}

void ui::Separator() {

	windowdata* data = activewindow;

	if (data->visible) {

		if (data->content_offset_x != 0) {
			data->content_offset_x = 0;
			data->content_offset_y += panelsize;
		}
		Renderer2D::DrawQuad({ data->x, data->y + data->content_offset_y, data->z }, { data->width, -5.0f }, data->tab_color, { 0.5f, -0.5f });

		data->content_offset_y += 5;
	}
}


void ui::InputBox(const char* id, std::string& text) {
	windowdata* data = activewindow;

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		uint32_t hashid = HashString(id);
		InputState& state = uiInputStates[hashid];

		if (state.text.empty() && !state.active)
			state.text = text;

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				activeelementid = hashid;
				state.active = !state.active;
				state.text = text;
				state.type = InputType::TEXT;
				was_mouse_pressed = true;
				EditorLayer::camera_locked = true;
			}
		}
		if (state.active && activeelementid == hashid) {
			Renderer2D::DrawQuad({ data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, data->z }, { width, -height }, glm::vec4(1.0f), { 0.5f, -0.5f });
			text = state.text;
		}
		else {
			state.active = false;
			Renderer2D::DrawQuad({ data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, data->z }, { width, -height }, glm::vec4(0.8f), { 0.5f, -0.5f });
		}

		fr->PrintStringui(state.text.c_str(), data->x + last_content_offsetx + 5 + width / 2, data->y + last_content_offsety + 5 + height / 2, data->z, (float)fontsize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}

}


void ui::StateButton(const char* text, bool& value, bool value2) {
	windowdata* data = activewindow;

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				value = value2;
				was_mouse_pressed = true;
				activeelementid = 0;
			}
		}

		float x = (float)(data->x + last_content_offsetx + 5);
		float y = (float)(data->y + last_content_offsety + 5);

		if (value == value2) {
			Renderer2D::DrawQuad({ x, y, data->z }, { width, -height }, data->tab_color, { 0.5f, -0.5f });
		}
		else {
			Renderer2D::DrawQuad({ x, y, data->z }, { width, -height }, data->panel_color + glm::vec4(0.1f, 0.1f, 0.1f, 0.0f), { 0.5f, -0.5f });
		}
		fr->PrintStringui(text, x + width / 2, y + height / 2, data->z, (float)fontsize);
	}
}

void ui::Label(const char* text) {
	windowdata* data = activewindow;

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		fr->PrintStringui(text, data->x + last_content_offsetx + 5 + width / 2, data->y + last_content_offsety + 5 +  height / 2, data->z, (float)fontsize);

	}
}


void ui::FloatInputBox(const char* id, float& value, float onclickvalue) {
	windowdata* data = activewindow;

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		uint32_t hashid = HashString(id);
		InputState& state = uiInputStates[hashid];

		if (state.text.empty() && !state.active)
			state.text = std::to_string(value);

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				activeelementid = hashid;
				state.active = !state.active;
				state.type = InputType::FLOAT;
				was_mouse_pressed = true;
				EditorLayer::camera_locked = true;
			}

			else if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_2) && !was_mouse_pressed) {
				value = onclickvalue;
				was_mouse_pressed = true;
				state.active = false;
				state.text = std::to_string(value);
				EditorLayer::camera_locked = true;
			}
		}

		float xpos = data->x + last_content_offsetx + 5;
		float ypos = data->y + last_content_offsety + 5;

		if (state.active && activeelementid == hashid)
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, -height }, glm::vec4(1.0f), { 0.5f, -0.5f });
		else {
			state.active = false;
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, -height }, glm::vec4(0.8f), { 0.5f, -0.5f });
		}

		fr->PrintStringui(state.text.c_str(), xpos + width / 2, ypos + height / 2, data->z, (float)fontsize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

		if (activeelementid == hashid) {
			value = atof(state.text.c_str());
		}
	}

}



// check box

bool ui::CheckBox(const char* text, bool& value) {
	windowdata* data = activewindow;

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 7);
		float ypos = (float)(data->y + last_content_offsety + 7);

		Renderer2D::DrawQuad({ xpos, ypos, data->z }, { height, -height }, checkboxboundary, glm::vec4(1.0f), { 0.5f, -0.5f });
		fr->PrintStringui(text, xpos + width / 2, ypos + height / 2, data->z, (float)fontsize);

		if (value) {
			Renderer2D::DrawQuad({ xpos + (height / 2), ypos + height / 2, data->z }, { (height * 0.7f), -height * 0.7f }, checkboxcenter, { 1.0f, 1.0f, 1.0f, 1.0f });
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


void ui::OptionSelector(std::vector<const char*> options, unsigned int& selectedoptionindex) {
	windowdata* data = activewindow;

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5);
		float ypos = (float)(data->y + last_content_offsety + 5);

		float xpos2 = (float)(data->x + inputwidth + last_content_offsetx);

		float textoffset = (float)(inputwidth / 2);

		Renderer2D::DrawQuad({ xpos, ypos, data->z }, { height, -height }, data->tab_color, { 0.5f, -0.5f });
		float trianglesize = height * 0.7f;

		Renderer2D::DrawQuad({ xpos2, ypos, data->z }, { height, -height }, data->tab_color, { -0.5f, -0.5f });

		fr->PrintStringui(options[selectedoptionindex], xpos + textoffset, ypos + height / 2, data->z, (float)fontsize);

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, 30, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				was_mouse_pressed = true;
				if (selectedoptionindex > 0)
					selectedoptionindex--;
			}
			Renderer2D::DrawRotatedTriangle({ xpos + height / 2, ypos + height / 2, data->z }, { trianglesize, trianglesize }, 90.0f, glm::vec4(1.0f));
		}

		else
			Renderer2D::DrawRotatedTriangle({ xpos + height / 2, ypos + height / 2, data->z }, { trianglesize, trianglesize }, 90.0f, data->panel_color);


		if (mousehover(data->x + inputwidth + last_content_offsetx - 30, data->y + last_content_offsety + 5, 30, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				was_mouse_pressed = true;
				if (selectedoptionindex < options.size() - 1)
					selectedoptionindex++;
			}
			Renderer2D::DrawRotatedTriangle({ xpos2 - height / 2, ypos + height / 2, data->z }, { trianglesize, trianglesize }, -90.0f, glm::vec4(1.0f));
		}
		else
			Renderer2D::DrawRotatedTriangle({ xpos2 - height / 2, ypos + height / 2, data->z }, { trianglesize, trianglesize }, -90.0f, data->panel_color);
	}
}


void ui::Image(SubTexture* image) {
	windowdata* data = activewindow;

	if (data->visible) {
		if (data->content_offset_x != 0) {
			Panel(panelsize);
		}

		float cwidth = image->coords2.x - image->coords1.x;
		float cheight = image->coords1.y - image->coords2.y;
		unsigned int index = image->index;


		auto s_texture = Renderer2D::textures[index];

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



			float x = (float)(data->x + data->content_offset_x);
			float y = (float)(data->y + data->content_offset_y);
			float yoffset = 200.0f;
			float xoffset = 200.0f;

			float wr = 400.0f;
			float hr = 400.0f;

			Renderer2D::DrawQuad({ x, y, data->z }, { 400, -400 }, data->panel_color, { 0.5f, -0.5f });
			Renderer2D::DrawQuad({ x + xoffset, y + yoffset, data->z }, { width * wr, height * hr }, *image);


			data->content_offset_y += 400;
		}
	}
}


void ui::UIntInputBox(const char* id, unsigned int& value, int onclickvalue) {
	windowdata* data = activewindow;

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5);
		float ypos = (float)(data->y + last_content_offsety + 5);

		uint32_t hashid = HashString(id);
		InputState& state = uiInputStates[hashid];

		if (state.text.empty() && !state.active) {
			state.text = std::to_string(value);
		}

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				was_mouse_pressed = true;
				activeelementid = hashid;
				state.active = !state.active;
				state.type = InputType::INT;
				state.text = std::to_string(value);
				EditorLayer::camera_locked = true;
			}

			else if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_2) && !was_mouse_pressed) {
				was_mouse_pressed = true;
				value = onclickvalue;
				state.active = false;
				activeelementid = 0;
				state.type = InputType::INT;
				state.text = std::to_string(value);
				EditorLayer::camera_locked = true;
			}
		}

		if (activeelementid != hashid)
			state.active = false;



		// if input box selected its highlighted
		if (state.active && activeelementid == hashid)
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, -height }, glm::vec4(1.0f), { 0.5f, -0.5f });
		else
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, -height }, glm::vec4(0.8f), { 0.5f, -0.5f });


		fr->PrintStringui(state.text.c_str(), xpos + width / 2, ypos + height / 2, data->z, (float)fontsize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

		if (state.active) {
			value = atof(state.text.c_str());
		}
	}

}


// for uint16


void ui::UInt16InputBox(const char* id, uint16_t& value, int onclickvalue) {
	windowdata* data = activewindow;

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;

		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5);
		float ypos = (float)(data->y + last_content_offsety + 5);

		uint32_t hashid = HashString(id);
		InputState& state = uiInputStates[hashid];

		if (state.text.empty() && !state.active)
			state.text = std::to_string(value);

		if (mousehover(data->x + last_content_offsetx + 5, data->y + last_content_offsety + 5, inputwidth, 30) && data->y + last_content_offsety + 5 > 0 && data->y + last_content_offsety + 5 < 1000) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) && !was_mouse_pressed) {
				was_mouse_pressed = true;
				activeelementid = hashid;
				state.active = !state.active;
				state.type = InputType::INT;
				state.text = std::to_string(value);
				EditorLayer::camera_locked = true;
			}

			else if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_2) && !was_mouse_pressed) {
				was_mouse_pressed = true;
				value = onclickvalue;
				state.active = false;
				activeelementid = 0;
				state.type = InputType::INT;
				state.text = std::to_string(value);
				EditorLayer::camera_locked = true;
			}
		}

		if (activeelementid != hashid) {
			state.active = false;
		}

		// if input box selected its highlighted
		if (state.active && activeelementid == hashid)
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, -height }, glm::vec4(1.0f), { 0.5f, -0.5f });
		else
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, -height }, glm::vec4(0.8f), { 0.5f, -0.5f });



		fr->PrintStringui(state.text.c_str(), xpos + width / 2, ypos + height / 2, data->z, (float)fontsize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		
		if (state.active) {
			value = atof(state.text.c_str());
		}
	}

}


bool ui::DropDownButton(const char* text, bool& value, unsigned int uioffset) {
	windowdata* data = activewindow;

	if (data->visible) {

		float width = 0.0f;
		float height = 0.0f;

		int inputwidth = 0;


		int last_content_offsetx = data->content_offset_x;
		int last_content_offsety = data->content_offset_y;

		updateuiparameters(data, inputwidth, last_content_offsetx, last_content_offsety, width, height);

		float xpos = (float)(data->x + last_content_offsetx + 5 + uioffset);
		float ypos = (float)(data->y + last_content_offsety + 5);

		float buttonwidth = 15.0f;
		float buttonheight = 15.0f;


		if (mousehover(data->x + last_content_offsetx + 5 + uioffset, data->y + last_content_offsety + 5, 30, 30)) {
			if(glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT) && !was_mouse_pressed)
				value = !value;
		}
	
		if (mousehover(data->x + last_content_offsetx + 35 + uioffset, data->y + last_content_offsety + 5, inputwidth - uioffset, 30)) {
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, -height }, data->panel_color - glm::vec4(.1f, .1f, .1f, .0f), { .5f, -.5f });
		}
		else {
			Renderer2D::DrawQuad({ xpos, ypos, data->z }, { width, -height }, data->panel_color, { .5f, -.5f });
		}

		if (value)
			Renderer2D::DrawRotatedTriangle({ xpos + height / 1.7f, ypos + height / 2, data->z }, { buttonwidth, buttonheight }, -180.0f, data->tab_color);
		else
			Renderer2D::DrawRotatedTriangle({ xpos + height / 1.7f, ypos + height / 2, data->z }, { buttonwidth, buttonheight }, -90.0f, data->tab_color);
		fr->PrintStringui(text, xpos + width / 2, ypos + height / 2, data->z, (float)fontsize);

		bool onclicked = false;
		if (mousehover(data->x + last_content_offsetx + 35 + uioffset, data->y + last_content_offsety + 5, inputwidth, 30) && glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT) && !was_mouse_pressed)
			onclicked = true;
		return onclicked;
	}
	return false;
}


bool ui::ImageButton(glm::ivec2 windowpos, glm::ivec2 size, SubTexture& image, float opacity) {

	Renderer2D::DrawQuad({windowpos.x, windowpos.y}, glm::vec2(size.x, -size.y), image, glm::vec4(1.0f, 1.0f, 1.0f, opacity), glm::vec2(0.5f, -0.5f));

	if (mousehover(windowpos.x, windowpos.y, size.x, size.y) && !was_mouse_pressed && glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT)) {
		return true;
	}

	return false;
}

unsigned int ui::RadioButtonGroup(const std::vector <const char*>& options, std::vector<bool> &checkoptions) {
	
	unsigned int len = options.size();

	
	return 0;
}

uint32_t ui::HashString(const char* str) {
	uint32_t hash = 2166136261u;
	while (*str) {
		hash = (hash ^ (uint8_t)*str++) * 16777619u;
	}
	return hash;
}
