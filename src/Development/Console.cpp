#include "Console.h"
#include "Editorlayer.h"
#include <Application.h>
#include <sstream>

#include <scripts/ObjectID.h>

#define selectedscene Application::s_serializer.m_scene

bool NewSceneLoaded = false;
std::string NewScenePath;

bool Console::OnEvent(Event& e) {

	uint32_t hashid = ui::HashString("console_command_box");
	InputState& state = ui::uiInputStates[hashid];

	if (e.is(Event_Type::Key_Pressed)) {
		KeyPressedEvent* ke = (KeyPressedEvent*)&e;

		int time = CommandStrings.size();
		int cy = y + time * 26;

		if (cy + 26 > Window::Height) {
			y += (cy + 26 - Window::Height);
		}

		
		if (ke->key == GLFW_KEY_ENTER && state.text.length() > 0) {
			CommandStrings.push_back(state.text);

			if (state.text == "clear") {
				ClearConsole();
			}

			// check size
			uint32_t size = state.text.size();

			if (size > 0) {
				std::stringstream ss(state.text);

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

			state.text = "";

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
			y += (cy + 26 - Window::Height);
		}

		if ((Window::Window_Mousex > 0 && Window::Window_Mousex < Window::Width) && (Window::Window_Mousey > cy && Window::Window_Mousey < cy + 26)) {
			state.active = true;
			state.type = InputType::TEXT;
			ui::activeelementid = hashid;
			return true;
		}

		if (ui::activeelementid != hashid)
			state.active = false;
	}
	return false;
}

void Console::Draw() {

	uint32_t hashid = ui::HashString("console_command_box");
	InputState& state = ui::uiInputStates[hashid];
	if (visible) {

		// if new scene loaded then load new scene in main scene
		if (NewSceneLoaded) {
			Application::GetInstance().loadsceneruntime(NewScenePath);
			NewSceneLoaded = false;
		}


		float width = Window::Width;

		// because width = window::orthographicsize * window::ratio
		float xpos = x;

		float ypos = y;

		int command_1_line_size = 26;
		int font_size = 20;
		float yoffset = command_1_line_size;

		int yintoffset = 0;

		unsigned int fontoffsetx = 10;
		for (auto str : CommandStrings) {

			int yintpos = y + yintoffset;

			Renderer2D::DrawQuad({ xpos, ypos, 1.0f }, { width, -yoffset }, BackGround_Color, { 0.5f, -0.5f });
			ui::fr->PrintStringui(str.c_str(), fontoffsetx, yintpos + font_size/2, 1.0f, font_size, glm::vec4(1.0f), true);
			
			ypos += yoffset;
			yintoffset += command_1_line_size;

		}

		Renderer2D::DrawQuad({ xpos, ypos, 1.0f }, { width, -yoffset }, Active_Line_Color, { 0.5f, -0.5f });
		if (state.active)
			ui::fr->PrintStringui(state.text.c_str(), fontoffsetx, yintoffset + font_size/2, 1.0f, font_size, glm::vec4(1.0f), true);
	}
}


void Console::ClearConsole() {
	CommandStrings.clear();
}



void Console::AddPlayer() {
	
}

void Console::AddPlayerFadingFrames() {

       
}
