#pragma once


#include <Renderer2D.h>
#include <UI/UI.h>
#include <Window.h>

struct Console {

	std::string currentstring = "";

	// color
	glm::vec4 BackGround_Color = glm::vec4(0.1f, 0.1f, 0.1f, 0.9f);
	glm::vec4 Font_Color = glm::vec4(1.0f);
	glm::vec4 Active_Line_Color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

	std::vector<std::string> CommandStrings;

	// position
	int x = 0, y = 0;

	bool visible = false;

	void Draw();

	bool OnEvent(Event& e);

	// command functions
	void ClearConsole();


	// this is for add player in scene quickly
	void AddPlayer();

};
