#pragma once


#include "EntityScript.h"
#include <Scene/Components.h>
#include <Scene/Scene.h>
#include <ParticleSystem.h>

struct PlayerCamera : EntityScript {

	void Start() override {

	}

	void Update(float ts) override {
		auto& transform = GetComponent<TransformComponent>();
		auto& tc = GetComponent<RelationshipComponent>().parentEntity->GetComponent<TransformComponent>();

		double x, y;
		glfwGetCursorPos(Window::ID, &x, &y);

		float offsetx = x - Window::Window_Last_Mousex;
		float offsety = y - Window::Window_Last_Mousey;

		if (offsetx != 0.0f || offsety != 0.0f) {
			Application::captureshadowmap = true;
			Application::GetInstance().m_scene->UpdateLightStatus(tc.position);
		}

		glm::vec3 newrot = transform.rotation;

		newrot.x += glm::degrees(offsety / Window::Height);
		newrot.y += glm::degrees(offsetx / Window::Width);

		transform.SetRotationEuler(newrot);

		ParticleSystem::yrotation = transform.rotation.y;
    }
};