#include "EntityScript.h"

#include <Scene/Components.h>

#include <WindowsInput.h>
#include <ShaderManager.h>

#include <Scene/SceneManager.h>

#include <Application.h>

#include "ObjectIndex.h"

struct Player : EntityScript {

	float speed = 5.0f;

	void Start() override {
		auto cameraentity = m_entity->m_scene->GetPrimaryCameraEntity();

		auto& ctc = cameraentity->GetComponent<TransformComponent>();

		ctc.rotation = { 0.0f, 0.0f, 0.0f };

		auto& pc = GetComponent<PhysicsComponent>();
		pc.m_rigidBody->setUserIndex(ObjectIndex::PLAYER);
	}

	void Update(float ts) override {

		auto& pc = GetComponent<PhysicsComponent>();
		auto& tc = GetComponent<TransformComponent>();

		glm::vec3 direction = pc.GetVelocity();
		
		auto cameraentity = m_entity->m_scene->GetPrimaryCameraEntity();

		auto& ctc = cameraentity->GetComponent<TransformComponent>();

		direction.x = -glm::sin(glm::radians(ctc.rotation.y));
		direction.z = -glm::cos(glm::radians(ctc.rotation.y));

		if (Input::KeyPressed(KEY_W)) {
			pc.SetVelocity(direction * speed);
			Application::captureshadowmap = true;
			Application::GetInstance().m_scene->UpdateLightStatus(tc.position);
		}

		else {
			glm::vec3 velocity = pc.GetVelocity();
			velocity.x = 0.0f;
			velocity.z = 0.0f;
			pc.SetVelocity(velocity);
		}
	}
};