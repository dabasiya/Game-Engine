#include "EntityScript.h"
#include <Scene/Components.h>
#include <Sound/SoundManager.h>

#include <Scene/AnimationManager.h>

#include "Enemy.h"


struct Pistol : EntityScript {

	bool canFire = true;

	glm::vec3 initialpos = glm::vec3(0.0f);

	double firetime = 0.0f;

	PhysicsSystem* m_physicssystem = nullptr;

	void Start() {
		initialpos = GetComponent<TransformComponent>().position;

		m_physicssystem = SceneManager::GetPhysicsSystem();
	}

	void Update(float ts) {

		if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT) && canFire) {
			auto smanager = SoundManager::GetInstance();
			canFire = false;
			smanager->PlayAudio("res/pistol_sound.mp3", AudioType::NO_LOOP);

			AnimationManager::RemoveOperation(m_entity);

			auto& tc = GetComponent<TransformComponent>();

			firetime = 0.0f;


			// shows particles and lights
			auto pe = GetComponent<RelationshipComponent>().childEntities[0];
			auto& pg = pe->GetComponent<ParticleGeneratorComponent>();
			auto& lc = pe->GetComponent<LightComponent>();
			lc.active = true;
			pg.active = true;


			// for gun animation by position interpolation
			tc.position = initialpos;

			glm::vec3 position1 = GetComponent<TransformComponent>().position;

			glm::vec3 position2 = position1 - glm::vec3(0.0f, 0.0f, -0.25f);

			std::vector<glm::vec3> positions = { position1, position2, position1 };

			AnimationOperation aop(positions, 0.25f, AnimationType::POSITION_INTERPOLATE, m_entity);
			AnimationManager::AddOperation(aop);

			// raycasting

			glm::vec3 from = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

			btVector3 fromb = btVector3(from.x, from.y, from.z);



			auto& ctc = m_entity->m_scene->GetPrimaryCameraEntity()->GetComponent<TransformComponent>();
			glm::vec3 to = ctc.direction() * 10.0f;

			btVector3 tob = fromb + btVector3(to.x, to.y, to.z);

			btCollisionWorld::ClosestRayResultCallback cb(fromb, tob);

			if (m_physicssystem) {
				m_physicssystem->getWorld()->rayTest(fromb, tob, cb);
			}

			if (cb.hasHit()) {
				int index = cb.m_collisionObject->getUserIndex();
				std::cout << index << std::endl;
				if (index == ObjectIndex::ENEMY) {

					Enemy* enemy = (Enemy*)cb.m_collisionObject->getUserPointer();
					std::cout << "enemy : " << (unsigned int)enemy->m_entity->id << std::endl;
					enemy->Died();
				}
				
			}
		}

		if (firetime > 0.25) {
			auto pe = GetComponent<RelationshipComponent>().childEntities[0];
			auto& pg = pe->GetComponent<ParticleGeneratorComponent>();
			auto& lc = pe->GetComponent<LightComponent>();
			pg.active = false;
		}

		if (firetime > 0.1f) {
			auto pe = GetComponent<RelationshipComponent>().childEntities[0];
			auto& lc = pe->GetComponent<LightComponent>();
			lc.active = false;
		}

		firetime += ts;
		
		if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_LEFT)) {

		}
		else {
			canFire = true;
		}
	}
};