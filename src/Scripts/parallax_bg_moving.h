#pragma once

#include "EntityScript.h"
#include <scene/Components.h>

struct parallax_bg_moving : EntityScript {

	Entity camera;

	Entity bg_1, bg_2, bg_3;

	glm::vec3 last_camerapos;

	void Start() override {
		auto view = m_entity.m_scene->m_registry.view<CameraComponent>();
		for (auto e : view) {
			auto& cc = view.get<CameraComponent>(e);
			if (cc.Primary == true) {
				camera = { e, m_entity.m_scene };
				break;
			}
		}

		auto view2 = m_entity.m_scene->m_registry.view<NameComponent>();
		for (auto e : view2) {
			auto& nc = view2.get<NameComponent>(e);

			if (nc.name == "bg_1") {
				bg_1 = { e, m_entity.m_scene };
			}

			else if (nc.name == "bg_2") {
				bg_2 = { e, m_entity.m_scene };
			}

			else if (nc.name == "bg_3") {
				bg_3 = { e, m_entity.m_scene };
			}
		}

		last_camerapos = camera.GetComponent<TransformComponent>().position;

	}

	void Update(float time) {
		glm::vec3 current_camerapos = camera.GetComponent<TransformComponent>().position;


		glm::vec3 diff = current_camerapos - last_camerapos;

		glm::vec3& bg_1_pos = bg_1.GetComponent<TransformComponent>().position;
		glm::vec3& bg_2_pos = bg_2.GetComponent<TransformComponent>().position;
		glm::vec3& bg_3_pos = bg_3.GetComponent<TransformComponent>().position;

		bg_1_pos -= diff * 0.5f;
		bg_2_pos -= diff * 0.25f;
		bg_3_pos -= diff * 0.125f;

		last_camerapos = camera.GetComponent<TransformComponent>().position;
	}
};