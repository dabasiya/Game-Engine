#include "Console.h"
#include "Editorlayer.h"
#include <Application.h>
#include <sstream>

#include <scripts/ObjectID.h>

#define selectedscene Application::s_serializer.m_scene

bool NewSceneLoaded = false;
std::string NewScenePath;

bool Console::OnEvent(Event& e) {

	if (e.is(Event_Type::Key_Pressed)) {
		KeyPressedEvent* ke = (KeyPressedEvent*)&e;

		int time = CommandStrings.size();
		int cy = y + time * 26;

		if (cy + 26 > Window::Height) {
			y -= (cy + 26 - Window::Height);
		}

		if (ke->key == GLFW_KEY_ENTER && ui::activeinputbox.text.length() > 0) {
			CommandStrings.push_back(ui::activeinputbox.text);

			if (ui::activeinputbox.text == "clear") {
				ClearConsole();
			}

			// check size
			uint32_t size = ui::activeinputbox.text.size();

			if (size > 0) {
				std::stringstream ss(ui::activeinputbox.text);

				std::string command;
				ss >> command;

				// load command its used for load new scene
				if (command == "load") {
					ss >> NewScenePath;
					NewSceneLoaded = true;
				}

				if (command == "reload" || command == "reset") {
					NewScenePath = Application::GetInstance().s_serializer.filepath;
					NewSceneLoaded = true;
				}
 
				else if (command == "Add_Player") {
					AddPlayer();
				}

                                else if(command == "Add_Player_Fading_Frames") {
                                        AddPlayerFadingFrames();
                                }
			}

			ui::activeinputbox.text = "";

			return true;
		}

		if (ke->mod & GLFW_MOD_ALT) {


			if (ke->key == 'C') {
				visible = !visible;
				EditorLayer::camera_locked = !EditorLayer::camera_locked;
				return true;
			}
		}
	}

	else if (e.is(Event_Type::Mouse_Pressed)) {
		int time = CommandStrings.size();
		int cy = y + time * 26;

		if (cy + 26 > Window::Height) {
			y -= (cy + 26 - Window::Height);
		}

		if ((Window::Window_Mousex > 0 && Window::Window_Mousex < Window::Width) && (Window::Window_Mousey > cy && Window::Window_Mousey < cy + 26)) {
			ui::activeinputbox = { "console_command_box", inputtype::TEXT, "" };
			return true;
		}
	}
	return false;
}

void Console::Draw() {
	if (visible) {

		// if new scene loaded then load new scene in main scene
		if (NewSceneLoaded) {
			Application::GetInstance().loadsceneruntime(NewScenePath);
			NewSceneLoaded = false;
		}


		float width = Window::OrthographicSize * 0.75f * Window::Ratio;

		// because width = window::orthographicsize * window::ratio
		float xpos = ((float)x / Window::Width) * width;
		xpos -= (width) / 2;

		float ypos = ((float)y / Window::Height) * Window::OrthographicSize;
		ypos = -(ypos - Window::OrthographicSize / 2);

		int command_1_line_size = 26;
		int font_size = 20;
		float yoffset = ((float)command_1_line_size / Window::Height) * Window::OrthographicSize;

		int yintoffset = 0;

		unsigned int fontoffsetx = Window::Width * 0.125f + 5;
		for (auto str : CommandStrings) {

			int yintpos = y + yintoffset;

			Renderer2D::DrawQuad({ xpos, ypos, 1.0f }, { width, yoffset }, BackGround_Color, { 0.5f, -0.5f });
			ui::fr->PrintString(str, fontoffsetx, yintpos + 23, 1.0f, font_size);

			ypos -= yoffset;
			yintoffset += command_1_line_size;

		}

		Renderer2D::DrawQuad({ xpos, ypos, 1.0f }, { width, yoffset }, Active_Line_Color, { 0.5f, -0.5f });
		if (ui::activeinputbox.id == "console_command_box")
			ui::fr->PrintString(ui::activeinputbox.text, fontoffsetx, y + yintoffset + 23, 1.0f, font_size);
	}
}


void Console::ClearConsole() {
	CommandStrings.clear();
}



void Console::AddPlayer() {
	
}

void Console::AddPlayerFadingFrames() {

       
}
