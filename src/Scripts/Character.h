#pragma once


#include "EntityScript.h"
#include <Scene/Components.h>
#include <WindowsInput.h>

enum CHARACTERSTATE {
	CHARACTER_IDLE, 
	CHARACTER_WALKING
};

struct CharacterEntity : EntityScript {

	CHARACTERSTATE m_state = CHARACTER_IDLE;

	float speed = 30.0f;
	
	void Start() override {
	
	}

	void Update(float ts) override {

		auto& transform = m_entity->GetComponent<TransformComponent>();
		auto& pc = GetComponent<PhysicsComponent>();
		auto tc = transform;

		auto cameraentity = m_entity->m_scene->GetPrimaryCameraEntity();

		auto& cameratransform = cameraentity->GetComponent<TransformComponent>();

		if (Input::KeyPressed(KEY_W)) {
			glm::vec3 curvelocity = pc.GetVelocity();
			curvelocity.x = sin(glm::radians(cameratransform.rotation.y+180.0f)) * speed;
			curvelocity.z = cos(glm::radians(cameratransform.rotation.y+180.0f)) * speed;
			pc.applyForce(curvelocity);
		}

		else {
			glm::vec3 curvelocity = pc.GetVelocity();
			curvelocity.x = 0.0f;
			curvelocity.z = 0.0f;
			pc.SetVelocity(curvelocity);
		}

	}
};