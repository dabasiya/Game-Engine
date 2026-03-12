#include "EntityScript.h"
#include <Scene/SceneManager.h>

#include "ObjectIndex.h"

struct Door : EntityScript {



	std::shared_ptr<Entity> cameraentity, presse;
	PhysicsSystem* m_physicssystem = nullptr;


	void Start() override {

		cameraentity = m_entity->m_scene->GetPrimaryCameraEntity();
		presse = SceneManager::GetEntityByName("presse");

		m_physicssystem = SceneManager::GetPhysicsSystem();

		auto& pc = m_entity->GetComponent<PhysicsComponent>();
		pc.m_rigidBody->setUserIndex(ObjectIndex::DOOR);


		void* pointer = (EntityScript*)this;
		pc.m_rigidBody->setUserPointer(pointer);
		
	}

	void Update(float ts) override {


		auto& ctc = cameraentity->GetComponent<TransformComponent>();
		glm::vec3 dir = glm::normalize(ctc.direction());
		glm::vec3 position = ctc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec3 to = position + (dir * 2.0f);

	


		btVector3 fromb(position.x, position.y, position.z);


		btVector3 tob(to.x, to.y, to.z);
		btCollisionWorld::ClosestRayResultCallback cb(fromb, tob);

		if (m_physicssystem) {
			m_physicssystem->getWorld()->rayTest(fromb, tob, cb);
		}


		if (cb.hasHit()) {
			void *pointer = cb.m_collisionObject->getUserPointer();
			int index = cb.m_collisionObject->getUserIndex();


			if (index == ObjectIndex::DOOR) {
				EntityScript* edoor = (EntityScript*)pointer;
				Door* door = (Door*)edoor;
				auto& uif = presse->GetComponent<UIFontRendererComponent>();
				uif.opacity = 1.0f;
				if (door && Input::KeyPressed(KEY_E)) {

					auto& tc = door->GetComponent<TransformComponent>();
					tc.rotation.y += 90.0f;
				}
			}
			else {
				auto& uif = presse->GetComponent<UIFontRendererComponent>();
				uif.opacity = 0.0f;
			}
		}
		else {
			auto& uif = presse->GetComponent<UIFontRendererComponent>();
			uif.opacity = 0.0f;
		}
	}
};