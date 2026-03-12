#pragma once

#include "EntityScript.h"
#include <Scene/Components.h>
#include <WindowsInput.h>

struct Cube : EntityScript {

	glm::vec3 force = glm::vec3(10.0f);
	float speed = 2.0f;

	void Start() override {

	}

	void Update(float ts) {

		PhysicsComponent& pc = GetComponent<PhysicsComponent>();
		
		if(Input::KeyPressed(KEY_W))
			pc.applyForce(force);

	}
};