#pragma once


#include "EntityScript.h"
#include <Scene/Components.h>

struct FPS : EntityScript {

	const std::string fpsstr = "FPS : ";

	void Start() {

	}

	void Update(float ts) {
		auto& fc = GetComponent<FontRendererComponent>();
		unsigned int fps = (unsigned int)(1.0f / ts);
		std::string newstring = fpsstr + std::to_string(fps);
		fc.text = newstring;
	}
};