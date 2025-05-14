#include "entityscript.h"


struct PlayerFadingFrames : EntityScript {

	void Start() {

	}

	void Update(float time) override {

		auto& spc = GetComponent<SpriteRendererComponent>();
		if (spc.opacity > 0.0f)
			spc.opacity -= time;
	}
};
