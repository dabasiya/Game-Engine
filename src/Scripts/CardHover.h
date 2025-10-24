#pragma once


#include "EntityScript.h"
#include <Scene/SceneManager.h>
#include <Scene/AnimationManager.h>


struct CardHover : EntityScript {

	bool mouse_pressed = false;

	std::vector<glm::vec4> colors = {
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
	};

	enum CARDPOSITION {
		BOTTOM = 0,
		LEFT,
		TOP,
		RIGHT
	};

	CARDPOSITION m_CardPosition;

	float finalc = -3.6165f;

	float hovertime = 0.5f;

	float ds = 0.15f;

	float speed = 5.0f;

	void Start() {
		auto& tc = GetComponent<TransformComponent>();

		if (tc.position.y < -2.5f) {
			m_CardPosition = BOTTOM;
			finalc = -3.6165f;
			tc.position.y = finalc;
		}
		else if (tc.position.y > 2.70f) {
			m_CardPosition = TOP;
			finalc = 3.5681f;
			tc.position.y = finalc;
		}
		else if (tc.position.x > 5.30f) {
			m_CardPosition = RIGHT;
			finalc = 6.1150f;
			tc.position.x = finalc;
		}
		else if (tc.position.x < -5.60f) {
			m_CardPosition = LEFT;
			finalc = -6.4016f;
			tc.position.x = finalc;
		}
	}

	void Update(float ts) {

		auto& tc = GetComponent<TransformComponent>();
		
		if (SceneManager::IsMouseHoveredOnEntity(m_entity)) {
			if (m_CardPosition == BOTTOM) {
				if (tc.position.y <= finalc + ds) {
					tc.position.y += ds * ts * speed;
				}
			}

			else if (m_CardPosition == TOP) {
				if (tc.position.y >= finalc - ds) {
					tc.position.y -= ds * ts * speed;
				}
			}

			else if (m_CardPosition == LEFT) {
				if (tc.position.x <= finalc + ds) {
					tc.position.x += ds * ts * speed;
				}
			}

			else if (m_CardPosition == RIGHT) {
				if (tc.position.x >= finalc - ds) {
					tc.position.x -= ds * ts * speed;
				}
			}

			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT) && !mouse_pressed) {
				auto& nc = GetComponent<NameComponent>();
				std::shared_ptr<Entity> cardnameentity = SceneManager::GetEntityByName("cardname");
				auto& cfc = cardnameentity->GetComponent<FontRendererComponent>();
				cfc.text = nc.name;
				cfc.opacity = 1.0f;

				std::shared_ptr<Entity> CardCloseButton = SceneManager::GetEntityByName("CardCloseButton");
				AnimationOperation opcls = AnimationOperation(0.0f, 1.0f, 0.5f, OPACITY_INTERPOLATE, CardCloseButton);
				AnimationManager::AddOperation(opcls);

				std::shared_ptr<Entity> cardborder = SceneManager::GetEntityByName("cardborder");
				AnimationOperation opcardborder = AnimationOperation(0.0f, 1.0f, 0.5f, OPACITY_INTERPOLATE, cardborder);
				AnimationManager::AddOperation(opcardborder);

				std::shared_ptr<Entity> cardbgentity = SceneManager::GetEntityByName("cardbackground");

				if (m_entity->HasComponent<CardComponent>()) {
					auto& cc = GetComponent<CardComponent>();
					auto& spc = cardbgentity->GetComponent<SpriteRendererComponent>();
					spc.color = colors[(unsigned int)cc.m_Cardtype];
					spc.color.a = 0.0f;
					AnimationOperation opcardbg = AnimationOperation(0.0f, 1.0f, 0.5f, OPACITY_INTERPOLATE, cardbgentity);
					AnimationManager::AddOperation(opcardbg);
				}
			}
		}
		else {
			if (m_CardPosition == BOTTOM) {
				if (tc.position.y > finalc) {
					tc.position.y -= ds * ts * speed;
				}
			}

			else if (m_CardPosition == TOP) {
				if (tc.position.y < finalc) {
					tc.position.y += ds * ts * speed;
				}
			}

			else if (m_CardPosition == LEFT) {
				if (tc.position.x > finalc) {
					tc.position.x -= ds * ts * speed;
				}
			}

			else if (m_CardPosition == RIGHT) {
				if (tc.position.x < finalc) {
					tc.position.x += ds * ts * speed;
				}
			}
		}

		if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT))
			mouse_pressed = true;
		else
			mouse_pressed = false;
	}
};