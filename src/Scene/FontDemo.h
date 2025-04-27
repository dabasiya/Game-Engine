#include "entityscript.h"

struct FontDemo : EntityScript {

	float multiplier = 1.0f;

	void start() override {
		auto& fontc = getcomponent<fontrenderercomponent>();
	}

	void update(float f) override {
		auto& fontc = getcomponent<fontrenderercomponent>();
		fontc.opacity += f * multiplier;
		if (fontc.opacity > 1.0f)
			multiplier = -1.0f;
		else if (fontc.opacity < 0.0f)
			multiplier = 1.0f;
	}
};
