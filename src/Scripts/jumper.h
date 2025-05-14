#pragma once

#include "EntityScript.h"
#include <Scene/Components.h>
#include "ContactListener.h"

struct jumper : EntityScript {


	Entity player;

	float timer = 0.0f;

	// time for jumper machine give jumps in seconds
	float jump_after_time = 3.0;

	void Start() override {

		auto view = m_entity.m_scene->m_registry.view<NameComponent>();

		for (auto e : view) {
			auto& nc = view.get<NameComponent>(e);
			if (nc.name == "Player") {
				player = { e, m_entity.m_scene };
				break;
			}
		}
	}

	void Update(float time) override {

		if (timer > jump_after_time) {

			if (cl.isOnJumper()) {
				auto& rb2d = player.GetComponent<RigidBody2DComponent>();
				rb2d.body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, 1.0f), true);
			}
			timer = timer - jump_after_time;

		}

		timer += time;
	}
};