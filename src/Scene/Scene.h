#pragma once

#include <entt.hpp>

#include <string>

#include <Event/Event.h>

#include <glm/glm.hpp>
#include <box2d/box2d.h>

#include <Scene/Setting/MainSetting.h>

#include "MessageBatch.h"

#include <Scene/PhysicsSystem.h>

#include <Model.h>


struct Entity;

struct Scene : public std::enable_shared_from_this<Scene> {


	// this is for render quad
	float vertices[20] = {
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f
	};

	unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };

	VBO gvbo;
	VAO gvao;
	EBO gebo;

	Scene(); 

	static MainSetting s_MainSetting;


	static MessageBatch s_msgbatch;

	static glm::vec3 cameraorientation;
	// this is for rendering sequence
	std::vector<std::shared_ptr<Entity>> entities;

	Model* cubemodel;

	void reorder_rendering_sequence();

	// 2d and 3d physics world
	b2World* physicsworld;

	PhysicsSystem* m_PhysicsSystem;


	std::shared_ptr<Entity> CreateEntity(const std::string& name = "Entity");
	void DestroyEntity(std::shared_ptr<Entity> a_entity);

	void SetChildOf(std::shared_ptr<Entity> parent, std::shared_ptr<Entity> child);

	bool PrimaryCameraExist();

	std::shared_ptr<Entity> GetPrimaryCameraEntity();

	
	void OnRuntimeStart();
	void OnRuntimeStop();

	bool OnEvent(Event& e);

	void Render3DModels(std::shared_ptr<Shader>& shader, glm::mat4 viewproj);

	void Update(float ts);

	void UpdateGlobalTransform(std::shared_ptr<Entity> entity, glm::mat4 transform);

	entt::registry m_registry;
};
