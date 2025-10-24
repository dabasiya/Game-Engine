#pragma once


#include "EntityScript.h"
#include <Scene/SceneManager.h>
#include <Scene/AnimationManager.h>


struct CardCloseButton : EntityScript {

	std::shared_ptr<Entity> cardbackground, cardname, cardborder;

	void Start() {
		cardbackground = SceneManager::GetEntityByName("cardbackground");
		cardname = SceneManager::GetEntityByName("cardname");
		cardborder = SceneManager::GetEntityByName("cardborder");
	}

	void Update(float ts) {

		
		if (SceneManager::IsMouseHoveredOnEntity(m_entity) && glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT)) {
			//auto& cbg_spc = cardbackground.GetComponent<SpriteRendererComponent>();
			auto& cn_spc = cardname->GetComponent<FontRendererComponent>();

			AnimationOperation ao = AnimationOperation(1.0f, 0.0f, 0.5f, OPACITY_INTERPOLATE, m_entity);
			AnimationManager::AddOperation(ao);

			AnimationOperation bo = AnimationOperation(1.0f, 0.0f, 0.5f, OPACITY_INTERPOLATE, cardbackground);
			AnimationManager::AddOperation(bo);
			std::vector<glm::vec3> pts = {
				{0.0f,0.0f,0.4f},
				{0.0f, -5.0f, 0.4f}
			};
			

			cn_spc.opacity = 0.0f;

			AnimationOperation cbo = AnimationOperation(1.0f, 0.0f, 0.5f, OPACITY_INTERPOLATE, cardborder);
			AnimationManager::AddOperation(cbo);
		}
	}
};