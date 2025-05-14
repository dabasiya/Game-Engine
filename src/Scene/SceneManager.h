#pragma once

#include <Application.h>
#include "Entity.h"

#include "Components.h"
struct SceneManager {

	static Entity GetEntityByName(const std::string& name) {
		auto view = Application::s_serializer.m_scene->m_registry.view<NameComponent>();
		Entity ent = { (entt::entity)0, nullptr };
		for (auto e : view) {
			auto& nc = view.get<NameComponent>(e);
			if (nc.name == name) {
				ent = { e, Application::s_serializer.m_scene };
				std::cout << "worked" << std::endl;
				break;
			}
		}
		return ent;
	}
};
