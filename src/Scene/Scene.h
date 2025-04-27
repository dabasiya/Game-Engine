#pragma once

#include <entt.hpp>

#include <string>

#include <Event/Event.h>

#include <glm/glm.hpp>
#include <box2d/box2d.h>

#include <Scene/Setting/MainSetting.h>


struct Entity;

struct Scene {

	static MainSetting s_MainSetting;

	// this is for rendering sequence
	std::vector<Entity> entities;

	void reorder_rendering_sequence();

	b2World* physicsworld;
	Entity CreateEntity(const std::string& name = "Entity");
	void DestroyEntity(Entity a_entity);


	void OnRuntimeStart();
	void OnRuntimeStop();

	bool OnEvent(Event& e);

	void Update(float ts);

	entt::registry m_registry;
};
