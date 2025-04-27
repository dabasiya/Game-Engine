#include "entityscript.h"

struct playerfollowcamera : EntityScript {

	Entity playerentity;

	void Start() override {
		auto view = m_entity.m_scene->m_registry.view<NameComponent>();

		for (auto e : view) {
			auto& nc = view.get<NameComponent>(e);
			if (nc.name == "player") {
				playerentity = { e, m_entity.m_scene };
				break;
			}
		}
	}

	void Update(float time) override {
		glm::vec3& cameraposition = m_entity.GetComponent<TransformComponent>().position;
		glm::vec3& playerposition = playerentity.GetComponent<TransformComponent>().position;

		float xdiff = playerposition.x - cameraposition.x;
		float ydiff = playerposition.y - cameraposition.y;

		cameraposition.x += xdiff * 0.1f;

                auto& cc = GetComponent<CameraComponent>();

                if(cameraposition.x < cc.x_min)
                        cameraposition.x = cc.x_min;

                else if(cameraposition.x > cc.x_max)
                        cameraposition.x = cc.x_max;

                if(cameraposition.y < cc.y_min)
                        cameraposition.y = cc.y_min;

                else if(cameraposition.y > cc.y_max)
                        cameraposition.y = cc.y_max;
	}
};
