
#include <Renderer2D.h>
#include "Entity.h"
#include "Components.h"
#include <Window.h>
#include <WindowsInput.h>
#include <Development/Editorlayer.h>
#include <Scripts/ScriptManager.h>
#include <Application.h>

#include <iostream>

#include <ParticleSystem.h>
// for create Entity
// this function register Entity in entt registry and return Entity structure

MainSetting Scene::s_MainSetting;

ParticleProps tpp = {};

ParticleSystem tps;

Entity Scene::CreateEntity(const std::string& name) {
	Entity e = { m_registry.create(), this };
	e.AddComponent<NameComponent>(name);
	e.AddComponent<TransformComponent>(glm::vec3(0.0f));
	return e;
}

// for delete Entity from entt registry
void Scene::DestroyEntity(Entity a_Entity) {
	m_registry.destroy(a_Entity.id);
}


void Scene::reorder_rendering_sequence() {

	entities.clear();

	std::vector<Entity> ents;
	auto view = m_registry.view<TransformComponent>();

	for (auto e : view) {
		Entity e1 = { e, this };
		if (e1.HasComponent<SpriteRendererComponent>() || e1.HasComponent<FontRendererComponent>() || e1.HasComponent<ParticleGeneratorComponent>())
			ents.push_back(e1);
	}

	std::vector<float> zs;

	for (auto e : ents) {
		auto t = e.GetComponent<TransformComponent>();
		zs.push_back(t.position.z);
	}

	for (int i = 0; i < zs.size(); i++) {
		for (int j = i + 1; j < zs.size(); j++) {
			if (zs[i] > zs[j]) {
				float temp = zs[j];
				zs[j] = zs[i];
				zs[i] = temp;

				Entity te = ents[j];
				ents[j] = ents[i];
				ents[i] = te;
			}
		}
	}

	for (auto e : ents)
		entities.push_back(e);
}

void Scene::OnRuntimeStart() {
	tpp.position = { 0.0f, 0.0f, 0.5f };
	tpp.velocity = { 0.0f, 0.0f };
	tpp.velocityvariation = { 0.1f, 0.5f };
	tpp.colorbegin = glm::vec4(1.0f);
	tpp.colorend = glm::vec4(glm::vec3(0.0f), 1.0f);
	tpp.sizebegin = 1.0f;
	tpp.sizeend = 0.2f;


	physicsworld = new b2World({ 0.0f, -9.0f });

	auto view = m_registry.view<RigidBody2DComponent>();
	for (auto e : view) {
		Entity e1 = { e, this };


		auto& transform = e1.GetComponent<TransformComponent>();
		auto& rb2d = e1.GetComponent<RigidBody2DComponent>();

		b2BodyDef bodydef;
		bodydef.type = rb2d.type;
		bodydef.position.Set(transform.position.x, transform.position.y);
		bodydef.fixedRotation = rb2d.fixedrotation;
		b2Body* body = physicsworld->CreateBody(&bodydef);
		rb2d.body = body;
		body->SetuintData((unsigned int)e);

		b2Vec2 pos = body->GetPosition();
		body->SetTransform(pos, transform.rotation.z * (3.14f / 180.0f));

		if (e1.HasComponent<BoxCollider2DComponent>()) {
			auto& bc2d = e1.GetComponent<BoxCollider2DComponent>();

			b2PolygonShape polygonshape;
			polygonshape.SetAsBox(bc2d.size.x * bc2d.width, bc2d.size.y * bc2d.height);

			b2FixtureDef fixturedef;
			fixturedef.shape = &polygonshape;
			fixturedef.density = bc2d.density;
			fixturedef.friction = bc2d.friction;
			fixturedef.isSensor = bc2d.isSensor;
			bc2d.fixture = body->CreateFixture(&fixturedef);
			bc2d.fixture->SetUintData(bc2d.objectid);
		}

		if (e1.HasComponent<ChainShapeColliderComponent>()) {
			auto& cs = e1.GetComponent<ChainShapeColliderComponent>();

			b2ChainShape chainshape;
			b2Vec2 prev = b2Vec2(0.0f, 0.0f);
			b2Vec2 next = b2Vec2(0.0f, 0.0f);
			unsigned int chainsize = cs.vertices.size();

			if (chainsize > 0) {
				prev = cs.vertices[0];
				prev.x += 0.1f;
				next = cs.vertices[chainsize - 1];
				next.x -= 0.1f;
			}
			chainshape.CreateChain(cs.vertices.data(), chainsize, prev, next);

			b2FixtureDef fixturedef;
			fixturedef.shape = &chainshape;
			fixturedef.density = 0.0f;
			cs.fixture = body->CreateFixture(&fixturedef);
			cs.fixture->SetUintData(cs.objectid);
		}
	}
}

void Scene::OnRuntimeStop() {
	delete physicsworld;
	physicsworld = nullptr;

	auto view = m_registry.view<ScriptComponent>();

	for (auto e : view) {
		auto& sc = view.get<ScriptComponent>(e);
		sc.destroy();
	}
}

bool Scene::OnEvent(Event& e) {

	if (e.is(Event_Type::Window_Resized)) {
		auto view = m_registry.view<CameraComponent>();
		for (auto e1 : view) {
			auto& cc = view.get<CameraComponent>(e1);
			cc.camera.Update();
		}
	}

	if (e.is(Event_Type::Mouse_Scrolled)) {
		auto view = m_registry.view<ScriptComponent>();
		for (auto e1 : view) {
			auto& sc = view.get<ScriptComponent>(e1);
			if (sc.script != nullptr) {
				sc.script->OnEvent(e);
			}
		}
	}

	return false;
}


// this function Scene hierarchy entities
// and also do rendering of Scene hierarchy entities
void Scene::Update(float ts) {

	// find camera and set its projection and view matrix in cameramatrix variable
	// and later it used in render2d for rendering other stuff
	glm::mat4 cameramatrix = glm::mat4(1.0f);
	bool cameraexist = false;
	glm::vec3 camerapos;

	auto view = m_registry.view<TransformComponent, CameraComponent>();

	for (auto e : view) {

		auto& transform = view.get<TransformComponent>(e);
		auto& cc = view.get<CameraComponent>(e);



		if (cc.Primary) {
			cameraexist = true;

			// camera movement in editormode
			if (Application::EditorMode && !EditorLayer::camera_locked) {

				if (Input::KeyPressed(KEY_D))
					transform.position.x += Window::OrthographicSize * ts;
				if (Input::KeyPressed(KEY_A))
					transform.position.x -= Window::OrthographicSize * ts;
				if (Input::KeyPressed(KEY_W))
					transform.position.y += Window::OrthographicSize * ts;
				if (Input::KeyPressed(KEY_S))
					transform.position.y -= Window::OrthographicSize * ts;
			}

			cameramatrix = cc.camera.viewprojection * glm::inverse(transform.getmatrix());
			camerapos = transform.position;
			break;
		}
	}


	if (!Application::EditorMode) {
		auto group1 = m_registry.view<ScriptComponent>();
		for (auto e : group1) {
			auto& scc = group1.get<ScriptComponent>(e);

			if (scc.script == nullptr) {
				scc.instantiate();
				if (scc.script != nullptr) {
					scc.script->m_entity = { e, this };
					scc.script->Start();
				}
			}
			if (scc.script != nullptr)
				scc.script->Update(ts);
		}





		// physics
		{
			const int32_t velocityiteration = 8;
			const int32_t positioniteration = 3;

			physicsworld->Step(ts, velocityiteration, positioniteration);

			auto view = m_registry.view<RigidBody2DComponent>();

			for (auto e : view) {
				Entity e1 = { e, this };


				auto& rb2d = e1.GetComponent<RigidBody2DComponent>();
				auto& transform = e1.GetComponent<TransformComponent>();

				b2Body* body = (b2Body*)rb2d.body;

				auto& position = body->GetPosition();
				transform.position.x = position.x;
				transform.position.y = position.y;
				//transform.rotation.z = body->GetRotation();
			}
		}

	}




	// LATER REMOVED
	// ! Now it fetches TransformComponent from entt registry and later it fetches spriterenderercomponenet it includes colors and textures
	// if cameraexist in Scene hierarchy panel then start rendering
	// else rendering processs doesnt't happen
	if (cameraexist) {


		Renderer2D::BeginScene(cameramatrix);

		for (auto e : entities) {
			if (e.HasComponent<SpriteRendererComponent>() && e.GetComponent<NameComponent>().active) {
				auto& sc = e.GetComponent<SpriteRendererComponent>();
				auto transform = e.GetComponent<TransformComponent>();
				auto& nc = e.GetComponent<NameComponent>();
				if (sc.type == 0)
					Renderer2D::DrawQuad(transform.getmatrix(), sc.color);
				else if (sc.type == 1) {
					transform.scale.x *= sc.m_subtexture.X_Multiplier;
					transform.scale.y *= sc.m_subtexture.Y_Multiplier;
					Renderer2D::DrawQuad(transform.getmatrix(), sc.m_subtexture);
				}

			}
		}



		for (auto e : entities) {
			if (e.HasComponent<FontRendererComponent>()) {
				auto& fc = e.GetComponent<FontRendererComponent>();
				auto& transform = e.GetComponent<TransformComponent>();

				ui::fr->PrintString(fc.text, transform.position.x, transform.position.y, fc.pixelsize, { 1.0f, 1.0f, 1.0f, fc.opacity });
			}

		}

		Renderer2D::EndScene();


		if (!Application::EditorMode) {
			// renders particle first 
		// this renders particles
			for (auto e : entities) {
				if (e.HasComponent<ParticleGeneratorComponent>()) {
					auto& pg = e.GetComponent<ParticleGeneratorComponent>();
					if (pg.active) {
						auto& transform = e.GetComponent<TransformComponent>();
						pg.time += ts;
						if (pg.time > 1.0f / pg.count) {
							pg.m_particleprops.position = transform.position;
							pg.m_particleprops.isimage = true;
							tps.Emit(pg.m_particleprops);
							pg.time = 0.0f;
						}
					}

					else {
						pg.time = 0.0f;
					}
				}
			}
			tps.OnUpdate(ts);
			tps.OnRender(cameramatrix);
		}


		if (Application::EditorMode && EditorLayer::showcolliders) {
			auto view = m_registry.view<BoxCollider2DComponent>();
			Renderer2D::BeginScene(cameramatrix);
			for (auto e : view) {
				Entity et = { e, this };
				if (et.HasComponent<RigidBody2DComponent>()) {
					auto& transform = et.GetComponent<TransformComponent>();
					auto& bc = et.GetComponent<BoxCollider2DComponent>();
					auto& rc = et.GetComponent<RigidBody2DComponent>();

					Renderer2D::DrawRotatedQuad(glm::vec3(transform.position.x, transform.position.y, 0.01f), { bc.width, bc.height }, transform.rotation.z, glm::vec4(1.0f));

				}
			}
			Renderer2D::EndScene();
		}

		// if entity has particle generator then render particle generator logo
		if (Application::EditorMode) {
			auto view = m_registry.view<ParticleGeneratorComponent>();
			Renderer2D::BeginScene(cameramatrix);
			for (auto e : view) {
				Entity et = { e, this };
				auto& transform = et.GetComponent<TransformComponent>();
				Renderer2D::DrawQuad(transform.position, { 0.3f, 0.3f }, EditorLayer::particlegeneratorsubtexture);
			}
			Renderer2D::EndScene();
		}




		// show chainshape colliders
		if (Application::EditorMode) {

			Renderer2D::BeginScene(cameramatrix);

			auto view = m_registry.view<ChainShapeColliderComponent>();
			for (auto e : view) {
				Entity et = { e, this };
				if (et.HasComponent<RigidBody2DComponent>()) {
					auto& tc = et.GetComponent<TransformComponent>();
					auto& cs = et.GetComponent<ChainShapeColliderComponent>();

					unsigned int size = cs.vertices.size();
					if (size > 1) {
						for (int i = 0; i < size - 1; i++) {
							glm::vec2 p1 = { tc.position.x + cs.vertices[i].x, tc.position.y + cs.vertices[i].y };
							glm::vec2 p2 = { tc.position.x + cs.vertices[i + 1].x, tc.position.y + cs.vertices[i + 1].y };
							Renderer2D::DrawLine({ p1.x, p1.y, 0.1f }, { p2.x, p2.y, 0.1f }, glm::vec4(1.0f));
						}
					}

					for (auto& pos : cs.vertices) {
						Renderer2D::DrawQuad({ tc.position.x + pos.x, tc.position.y + pos.y, 0.1f }, { 0.05f, 0.05f }, { 0.0f, 0.0f, 1.0f, 1.0f });
					}

				}
			}

			Renderer2D::EndScene();


			if (EditorLayer::camera_locked) {
				Renderer2D::BeginScene(glm::ortho(-Window::Ratio, Window::Ratio, -1.0f, 1.0f));
				ui::fr->PrintString("Locked Camera", 0.0f, 0.5f, 64.0f);
				Renderer2D::EndScene();
			}

		}

		//s_MainSetting.Render();
	}

}
