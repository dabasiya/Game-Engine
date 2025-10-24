#pragma once

#include <Application.h>
#include "Entity.h"

#include "Components.h"
struct SceneManager {

	static std::shared_ptr<Entity> GetEntityByName(const std::string& name) {
		auto app = Application::GetInstance();
		auto view = app.s_serializer.m_scene->m_registry.view<NameComponent>();
		std::shared_ptr<Entity> ent = std::make_shared<Entity>();
		for (auto e : view) {
			auto& nc = view.get<NameComponent>(e);
			if (nc.name == name) {
				ent->id = e;
				ent->m_scene = app.s_serializer.m_scene;
				break;
			}
		}
		return ent;
	}


	// for 2d mouse is hovered on entity 
	static bool IsMouseHoveredOnEntity(std::shared_ptr<Entity> e) {
		std::shared_ptr<Entity> cameraEntity = Application::GetInstance().s_serializer.m_scene->GetPrimaryCameraEntity();

		auto& cameratransform = cameraEntity->GetComponent<TransformComponent>();
		auto& entitytransform = e->GetComponent<TransformComponent>();

		glm::vec2 mousecoords = { Window::Mousex, Window::Mousey };
		mousecoords += glm::vec2(cameratransform.position.x, cameratransform.position.y);

		float angle = -entitytransform.rotation.z;

		glm::vec2 d_coords = mousecoords - glm::vec2(entitytransform.position.x, entitytransform.position.y);

		glm::vec2 tempcoords = d_coords;
		d_coords.x = tempcoords.x * glm::cos(glm::radians(angle)) - tempcoords.y * glm::sin(glm::radians(angle));
		d_coords.y = tempcoords.x * glm::sin(glm::radians(angle)) + tempcoords.y * glm::cos(glm::radians(angle));

		glm::vec3 scale = entitytransform.scale;

		if (d_coords.x > -scale.x / 2 && d_coords.x < scale.x / 2) {
			if (d_coords.y > -scale.y / 2 && d_coords.y < scale.y / 2) {
				return true;
			}
		}
		return false;
	}
};
