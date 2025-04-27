#pragma once

#include "EntityScript.h"
#include <scene/Components.h>

struct movingpgtest : EntityScript {

	bool rightmove = true;

	float dir_change_time = 2.0f;

	float timer = 0.0f;

	void Update(float time) override {
		auto& tc = GetComponent<TransformComponent>();
		
		if (timer > dir_change_time) {
			rightmove = !rightmove;

			timer = timer - dir_change_time;
		}

		if (rightmove)
			tc.position.x += time * 3.0f;
		else
			tc.position.x -= time * 3.0f;

		timer += time;
	}
};