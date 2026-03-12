#pragma once


#include "EntityScript.h"
#include <Scene/SceneManager.h>
#include "ObjectIndex.h"

struct Enemy : EntityScript {

	PhysicsSystem* m_physicssystem = nullptr;

	std::shared_ptr<Entity> PlayerEntity;

	float diedtime = 0.0f;
	bool died = false;

	void Start() override {
		m_physicssystem = SceneManager::GetPhysicsSystem();
		PlayerEntity = SceneManager::GetEntityByName("player");

		auto& pc = GetComponent<PhysicsComponent>();
		void* pointer = this;

		pc.m_rigidBody->setUserIndex(ObjectIndex::ENEMY);
		pc.m_rigidBody->setUserPointer(pointer);
	}

	void Update(float ts) override {

		auto& playertc = PlayerEntity->GetComponent<TransformComponent>();
		glm::vec3 to = glm::vec3(playertc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

		btVector3 tob = btVector3(to.x, to.y + 1.2f, to.z);

		auto& tc = GetComponent<TransformComponent>();
		glm::vec3 from = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

		btVector3 fromb = btVector3(from.x, from.y + 1.2f, from.z);

		btCollisionWorld::ClosestRayResultCallback cb(fromb, tob);

		if (m_physicssystem) {
			m_physicssystem->getWorld()->rayTest(fromb, tob, cb);
		}

		if (cb.hasHit()) {
			int index = cb.m_collisionObject->getUserIndex();
		}

		if (diedtime > 0.15f) {
			m_entity->m_scene->DestroyEntity(m_entity);
		}

		if (died)
			diedtime += ts;
	}

	void Died() {
		died = true;
		m_entity->RemoveComponent<Model3DComponent>();
		auto& pc = m_entity->GetComponent<PhysicsComponent>();
		btDiscreteDynamicsWorld* world = Application::GetInstance().s_serializer.m_scene->m_PhysicsSystem->getWorld();
		pc.destroyRuntimeBody(world);
		m_entity->RemoveComponent<PhysicsComponent>();
		auto& pg = GetComponent<ParticleGeneratorComponent>();
		pg.active = true;
		auto& tc = m_entity->GetComponent<TransformComponent>();
		glm::vec3 position = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		m_entity->m_scene->UpdateLightStatus(position);
	}
};