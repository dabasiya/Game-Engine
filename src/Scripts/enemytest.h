#pragma once

#include "EntityScript.h"
#include <scene/Components.h>
#include "ContactListener.h"
#include <Application.h>


struct enemytest : EntityScript {

	float enemyspeed = 50.0f;

	float rotation = 0.0f;

	bool isdied = false;

	void Start() override {

	}

	void Update(float time) override {
		auto& rb2d = GetComponent<RigidBody2DComponent>();
		auto& bc2d = GetComponent<BoxCollider2DComponent>();

		b2Body* body = rb2d.body;

		b2Vec2 currentvelocity = body->GetLinearVelocity();

		body->SetLinearVelocity(b2Vec2(enemyspeed * time, currentvelocity.y));

		if (cl.is_enemy_died && cl.isEnemydied((unsigned int)m_entity.id) && !isdied) {
			bc2d.fixture->SetSensor(true);
			isdied = true;
			Application::s_SoundManager->PlayAudio("res/enemydie.mp3", NO_LOOP);
		}

		if (isdied) {
			auto& transform = GetComponent<TransformComponent>();
			rotation += time / 2.0f;
			transform.rotation.z += rotation * 10.0f;
		}

	}
};
