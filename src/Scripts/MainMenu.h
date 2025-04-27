#include "entityscript.h"
#include <scene/components.h>
#include <windowsinput.h>

#include <application.h>

struct MainMenu : EntityScript {
	unsigned int selected_option = 0;

	float change = -1.0f;

	std::string options[3] = {
	  "Play",
	  "Setting",
	  "About"
	};

	float minimum_opacity = 0.3f;

	bool keypressed = false;

	// not selected option opacity
	float unselopacity = 0.3f;

	Entity menu_options[3];

	void Start() override {

		auto view = m_entity.m_scene->m_registry.view<NameComponent>();

		for (auto e : view) {
			for (unsigned int i = 0; i < 3; i++) {
				std::string name = view.get<NameComponent>(e).name;
				if (name == options[i])
					menu_options[i] = { e, m_entity.m_scene };
			}
		}

		Application::s_SoundManager->PlayAudio("theme.mp3", LOOP);

	}

	void Update(float time) override {


		if (Input::KeyPressed(KEY_DOWN) && !keypressed) {
			if (selected_option < 2)
				selected_option++;
		}

		else if (Input::KeyPressed(KEY_UP) && !keypressed) {
			if (selected_option > 0)
				selected_option--;
		}

		for (unsigned int i = 0; i < 3; i++) {
			auto& fontcomp = menu_options[i].GetComponent<FontRendererComponent>();
			if (selected_option == i) {
				if (fontcomp.opacity >= 1.0f)
					change = -1.0f;
				else if (fontcomp.opacity <= minimum_opacity)
					change = 1.0f;

				fontcomp.opacity += change * time;
			}
			else {
				if (fontcomp.opacity < unselopacity)
					fontcomp.opacity += time;
				if (fontcomp.opacity > unselopacity)
					fontcomp.opacity = unselopacity;
			}
		}

		if (Input::KeyPressed(KEY_UP) || Input::KeyPressed(KEY_DOWN)) {
			keypressed = true;
			change = -1.0f;
		}
		else
			keypressed = false;

		if (Input::KeyPressed(KEY_ENTER)) {
			if (selected_option == 0) {
				Application::loadsceneruntime("res/test1.scene");
				Application::s_SoundManager->StopAudio("theme.mp3");
			}
		}
	}
};
