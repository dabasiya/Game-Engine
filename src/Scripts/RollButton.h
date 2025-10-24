#pragma once

#include "EntityScript.h"
#include <Scene/SceneManager.h>
#include <WindowsInput.h>
#include <random>

struct RollButton : EntityScript {

	float visualtime = 0.6f;

	float speed = 3.0f;

	bool visible = true;

	float time = 0.0f;

	bool canroll = true;

	unsigned dice1 = 1, dice2 = 1;

	bool rolling = false;

	bool undermouse = false;


	glm::vec2 normalscale = { 4.0f, 1.0f };
	glm::vec2 hoverscale = { 3.5f, 0.875f };

	// for random
	std::random_device rd;
	std::mt19937 gen;
	std::uniform_int_distribution<int> dist;

	void Start() {
		gen = std::mt19937(rd());
		dist = std::uniform_int_distribution<int>(1, 6);
	}

	void Update(float ts) {

		if (SceneManager::IsMouseHoveredOnEntity(m_entity)) {
			undermouse = true;
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT) && canroll) {
				visible = !visible;
				time = 0.0f;
				canroll = false;
				rolling = true;
				dice1 = dist(gen);
				dice2 = dist(gen);
			}
		}
		else {
			undermouse = false;
		}

		auto& tc = GetComponent<TransformComponent>();
		// change size if under mouse
		if (undermouse) {
			if (tc.scale.x > hoverscale.x && tc.scale.y > hoverscale.y) {
				tc.scale.x -= ts * 4.0f;
				tc.scale.y -= ts;
			}

			if (tc.scale.x < hoverscale.x || tc.scale.y < hoverscale.y) {
				tc.scale.x = hoverscale.x;
				tc.scale.y = hoverscale.y;
			}
		}
		else {
			if (tc.scale.x < normalscale.x && tc.scale.y < normalscale.y) {
				tc.scale.x += ts * 4.0f;
				tc.scale.y += ts;
			}

			if (tc.scale.x > normalscale.x || tc.scale.y > normalscale.y) {
				tc.scale.x = normalscale.x;
				tc.scale.y = normalscale.y;
			}
		}
	
		time += ts;

		auto& spc = GetComponent<SpriteRendererComponent>();
		if (time > 1.0f && spc.opacity < 1.0f) {
			spc.opacity += speed * ts;
		}
		else if (time < 1.0f && spc.opacity > 0.0f) {
			spc.opacity -= speed * ts;
		}

		if (spc.opacity > 1.0f)
			spc.opacity = 1.0f;
		else if (spc.opacity < 0.0f)
			spc.opacity = 0.0f;

		if (time > 1.2f) {
			rolling = false;
		}
		if (time > 2.0f) {
			canroll = true;
		}
	}
};