
#include <Renderer2D.h>
#include "Entity.h"
#include "Components.h"
#include <Window.h>
#include <WindowsInput.h>
#include <Development/Editorlayer.h>
#include <Scripts/ScriptManager.h>
#include <Application.h>
#include <Scene/AnimationManager.h>
#include <ModelManager.h>

#include <ShaderManager.h>

#include <iostream>

#include <ParticleSystem.h>

float farplane = 2.0f;
// for create Entity
// this function register Entity in entt registry and return Entity structure

MainSetting Scene::s_MainSetting;

MessageBatch Scene::s_msgbatch;

ParticleProps tpp = {};

ParticleSystem tps;




float lastmousex = Window::Width / 2;
float lastmousey = Window::Height / 2;


glm::vec3 Scene::cameraorientation = glm::vec3(1.0f);



#define AppScene Application::GetInstance().s_serializer.m_scene

std::shared_ptr<Entity> Scene::CreateEntity(const std::string& name) {
	std::shared_ptr<Entity> e = std::make_shared<Entity>();
	e->m_scene = shared_from_this();
	e->id = m_registry.create();
	e->AddComponent<NameComponent>(name);
	e->AddComponent<RelationshipComponent>();
	e->AddComponent<TransformComponent>(glm::vec3(0.0f));
	return e;
}

// for delete Entity from entt registry
void Scene::DestroyEntity(std::shared_ptr<Entity> a_Entity) {
	
	// remove entity shared ptr from parents entities child lists
	auto& rc = a_Entity->GetComponent<RelationshipComponent>();

	if (rc.parentEntity) {
		auto& prc = rc.parentEntity->GetComponent<RelationshipComponent>();
		
		prc.childEntities.erase(std::remove(prc.childEntities.begin(), prc.childEntities.end(), a_Entity), prc.childEntities.end());
	}

	m_registry.destroy(a_Entity->id);
}

void Scene::SetChildOf(std::shared_ptr<Entity> parent, std::shared_ptr<Entity> child) {

	auto& prc = parent->GetComponent<RelationshipComponent>();
	auto& crc = parent->GetComponent<RelationshipComponent>();

	if (crc.parentEntity) {
		auto& oprc = crc.parentEntity->GetComponent<RelationshipComponent>();

		prc.childEntities.erase(std::remove(prc.childEntities.begin(), prc.childEntities.end(), child), prc.childEntities.end());
	}

	crc.parentEntity = parent;
	prc.childEntities.push_back(child);
}


void Scene::reorder_rendering_sequence() {

	entities.clear();

	std::vector<std::shared_ptr<Entity>> ents;
	auto view = m_registry.view<TransformComponent>();

	for (auto e : view) {
		std::shared_ptr<Entity> e1 = std::make_shared<Entity>();
		e1->id = e;
		e1->m_scene = shared_from_this();

		if (e1->HasComponent<SpriteRendererComponent>() || e1->HasComponent<FontRendererComponent>() || e1->HasComponent<ParticleGeneratorComponent>() || e1->HasComponent<Model3DComponent>())
			ents.push_back(e1);
	}

	std::vector<float> zs;

	for (auto e : ents) {
		auto t = e->GetComponent<TransformComponent>();
		zs.push_back(t.position.z);
	}

	for (int i = 0; i < zs.size(); i++) {
		for (int j = i + 1; j < zs.size(); j++) {
			if (zs[i] > zs[j]) {
				float temp = zs[j];
				zs[j] = zs[i];
				zs[i] = temp;

				std::shared_ptr<Entity> te = ents[j];
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

	// later removed
	m_PhysicsSystem = new PhysicsSystem();

	auto view = m_registry.view<RigidBody2DComponent>();
	for (auto e : view) {
		std::shared_ptr<Entity> e1 = std::make_shared<Entity>();

		auto& transform = e1->GetComponent<TransformComponent>();
		auto& rb2d = e1->GetComponent<RigidBody2DComponent>();

		rb2d.id = (unsigned int)e;

		b2BodyDef bodydef;
		bodydef.type = rb2d.type;
		bodydef.position.Set(transform.position.x, transform.position.y);
		bodydef.fixedRotation = rb2d.fixedrotation;
		b2Body* body = physicsworld->CreateBody(&bodydef);
		rb2d.body = body;
		body->SetuintData((unsigned int)e);

		b2Vec2 pos = body->GetPosition();
		body->SetTransform(pos, transform.rotation.z * (3.14f / 180.0f));

		if (e1->HasComponent<BoxCollider2DComponent>()) {
			auto& bc2d = e1->GetComponent<BoxCollider2DComponent>();

			b2PolygonShape polygonshape;
			if (bc2d.sameasscale)
				polygonshape.SetAsBox(bc2d.size.x * transform.scale.x, bc2d.size.y * transform.scale.y);
			else
				polygonshape.SetAsBox(bc2d.size.x * bc2d.width, bc2d.size.y * bc2d.height);

			b2FixtureDef fixturedef;
			fixturedef.shape = &polygonshape;
			fixturedef.density = bc2d.density;
			fixturedef.friction = bc2d.friction;
			fixturedef.isSensor = bc2d.isSensor;
			bc2d.fixture = body->CreateFixture(&fixturedef);
			bc2d.fixture->SetUintData(bc2d.objectid);
		}

		if (e1->HasComponent<ChainShapeColliderComponent>()) {
			auto& cs = e1->GetComponent<ChainShapeColliderComponent>();

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



	// create 3d physics components
	auto physicscomponentview = m_registry.view<PhysicsComponent>();

	for (auto e : physicscomponentview) {
		auto& pc = physicscomponentview.get<PhysicsComponent>(e);
		std::shared_ptr<Entity> entity = std::make_shared<Entity>();
		entity->id = e;
		entity->m_scene = shared_from_this();
		auto& tc = entity->GetComponent<TransformComponent>();

		pc.InitialPosition = tc.position;
		pc.InitialRotation = PhysicsComponent::EulerToQuat(tc.rotation);

		pc.initializeRuntimeBody(m_PhysicsSystem->getWorld());

		glm::quat rot = PhysicsComponent::EulerToQuat(tc.rotation);
		pc.updateGameTransform(tc.position, rot);
		tc.rotation = PhysicsComponent::QuatToEuler(rot);
	}


	// set camera orientations

	{
		auto e = GetPrimaryCameraEntity();

		if (e) {
			auto& tc = e->GetComponent<TransformComponent>();
			auto& cc = e->GetComponent<CameraComponent>();

			glm::vec3 rotation = tc.rotation;

			glm::vec3 orientation = glm::vec3(0.0f);

			orientation.x = -glm::sin(glm::radians(rotation.y)) * glm::cos(glm::radians(rotation.x));
			orientation.z = glm::cos(glm::radians(rotation.y + 180.0f)) * glm::cos(glm::radians(rotation.x));
			orientation.y = glm::sin(glm::radians(rotation.x));

			cameraorientation = orientation;
		}
	}
}

std::shared_ptr<Entity> Scene::GetPrimaryCameraEntity() {
	std::shared_ptr<Entity> et = std::make_shared<Entity>();
	auto view = m_registry.view<CameraComponent>();

	for (auto e : view) {
		auto& cc = view.get<CameraComponent>(e);

		if (cc.Primary) {
			et->id = e;
			et->m_scene = shared_from_this();
			return et;
		}
	}

	return nullptr;
}

bool Scene::PrimaryCameraExist() {
	auto view = m_registry.view<CameraComponent>();
	for (auto e : view) {
		auto& cc = view.get<CameraComponent>(e);

		if (cc.Primary)
			return true;
	}
	return false;
}

void Scene::OnRuntimeStop() {
	delete physicsworld;
	physicsworld = nullptr;

	auto view = m_registry.view<ScriptComponent>();

	for (auto e : view) {
		auto& sc = view.get<ScriptComponent>(e);
		sc.destroy();
	}

	// for bullet physics
	auto physicscomponentview = m_registry.view<PhysicsComponent>();

	for (auto e : physicscomponentview) {
		auto& pc = physicscomponentview.get<PhysicsComponent>(e);
		pc.destroyRuntimeBody(m_PhysicsSystem->getWorld());
	}
}

bool Scene::OnEvent(Event& e) {

	if (e.is(Event_Type::Window_Resized)) {
		auto view = m_registry.view<CameraComponent>();
		for (auto e1 : view) {
			auto& cc = view.get<CameraComponent>(e1);
			cc.camerao.Update();
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

	if (e.is(Event_Type::Mouse_Pressed)) {
		double x, y;
		glfwGetCursorPos(Window::ID, &x, &y);

		lastmousex = x;
		lastmousey = y;
	}

	return false;
}

// update globaltransform of transformcomponent of entity

void Scene::UpdateGlobalTransform(std::shared_ptr<Entity> entity, glm::mat4 transform) {

	auto& tc = entity->GetComponent<TransformComponent>();
	auto& rc = entity->GetComponent<RelationshipComponent>();
	
	tc.worldtransform = transform;

	glm::mat4 wt = tc.worldtransform * tc.getmatrix();

	for (auto e : rc.childEntities) {
		UpdateGlobalTransform(e, wt);
	}
}


// this function Scene hierarchy entities
// and also do rendering of Scene hierarchy entities
void Scene::Update(float ts) {

	// update world transform

	{
		auto view = m_registry.view<RelationshipComponent>();

		for (auto e : view) {
			std::shared_ptr<Entity> ent = std::make_shared<Entity>();
			ent->id = e;
			ent->m_scene = AppScene;

			auto& rc = ent->GetComponent<RelationshipComponent>();

			if (!rc.parentEntity) {
				UpdateGlobalTransform(ent, glm::mat4(1.0f));
			}
		}
	}


	auto& app = Application::GetInstance();


	// later removed

	float cameraz = 0.0f;

	s_msgbatch.Update(ts);

	bool cameraexist = false;
	glm::mat4 cameramatrix = glm::mat4(1.0f);
	glm::vec3 camerapos;
	// Editor Camera input and update 
	{
		// find camera and set its projection and view matrix in cameramatrix variable
		// and later it used in render2d for rendering other stuff

		auto view = m_registry.view<TransformComponent, CameraComponent>();

		for (auto e : view) {

			auto& transform = view.get<TransformComponent>(e);
			auto& cc = view.get<CameraComponent>(e);


			cameraz = transform.position.z;


			if (cc.Primary) {
				cameraexist = true;

				// camera movement in editormode
				if (app.EditorMode && !EditorLayer::camera_locked && cc.cameratype == CameraType::Orthographic) {

					if (Input::KeyPressed(KEY_D))
						transform.position.x += Window::OrthographicSize * ts;
					if (Input::KeyPressed(KEY_A))
						transform.position.x -= Window::OrthographicSize * ts;
					if (Input::KeyPressed(KEY_W))
						transform.position.y += Window::OrthographicSize * ts;
					if (Input::KeyPressed(KEY_S))
						transform.position.y -= Window::OrthographicSize * ts;
				}

				else if (app.EditorMode && !EditorLayer::camera_locked && cc.cameratype == CameraType::Perspective) {

					glm::vec3 rotation = transform.rotation;

					glm::vec3 orientation = glm::vec3(0.0f);

					orientation.x = -glm::sin(glm::radians(rotation.y)) * glm::cos(glm::radians(rotation.x));
					orientation.z = glm::cos(glm::radians(rotation.y + 180.0f)) * glm::cos(glm::radians(rotation.x));
					orientation.y = glm::sin(glm::radians(rotation.x));

					cameraorientation = orientation;

					if (Input::KeyPressed(KEY_W))
						transform.position += orientation * EditorLayer::EditorCameraSpeed;
					if (Input::KeyPressed(KEY_S))
						transform.position -= orientation * EditorLayer::EditorCameraSpeed;
					if (Input::KeyPressed(KEY_A)) {
						glm::vec3 a = glm::cross(orientation, glm::vec3(0.0f, 1.0f, 0.0f)) * EditorLayer::EditorCameraSpeed;
						transform.position -= a;
					}
					if (Input::KeyPressed(KEY_D)) {
						glm::vec3 a = glm::cross(orientation, glm::vec3(0.0f, 1.0f, 0.0f)) * EditorLayer::EditorCameraSpeed;
						transform.position += a;
					}
				}

				// when we develop 3d games
				if (app.EditorMode && !EditorLayer::camera_locked && EditorLayer::s_EditorCameraType == THREE_AXIS_MOVE_CAMERA) {

					if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_RIGHT)) {
						double x, y;
						glfwGetCursorPos(Window::ID, &x, &y);

						float offsetx = x - lastmousex;
						float offsety = y - lastmousey;

						transform.rotation.x += glm::degrees(offsety / Window::Height);
						transform.rotation.y += glm::degrees(offsetx / Window::Width);

						lastmousex = x;
						lastmousey = y;
					}
				}



				if (cc.cameratype == CameraType::Orthographic) {
					cameramatrix = cc.camerao.viewprojection * glm::inverse(transform.getmatrix());
				}
				else {
					cc.camerap.Update();
					cameramatrix = cc.camerap.viewprojection * glm::lookAt(transform.position, transform.position + cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
				}
				camerapos = transform.position;
				break;
			}
		}


	}


	// if EditorMode == false
	if (!app.EditorMode) {

		// Script Related Stuff
		{
			// for instantiate script

			auto group1 = m_registry.view<ScriptComponent>();
			for (auto e : group1) {
				auto& scc = group1.get<ScriptComponent>(e);

				if (scc.script == nullptr) {
					scc.instantiate();
					if (scc.script != nullptr) {
						scc.script->m_entity = std::make_shared<Entity>();
						scc.script->m_entity->id = e;
						scc.script->m_entity->m_scene = shared_from_this();
						scc.script->Start();
					}
				}
			}

			// for running
			for (auto e : group1) {
				auto& scc = group1.get<ScriptComponent>(e);

				if (scc.script != nullptr)
					scc.script->Update(ts);
			}

		}

		// physics
		{
			const int32_t velocityiteration = 8;
			const int32_t positioniteration = 2;

			physicsworld->Step(ts, velocityiteration, positioniteration);

			auto view = m_registry.view<RigidBody2DComponent>();

			for (auto e : view) {
				std::shared_ptr<Entity> e1 = std::make_shared<Entity>();
				e1->id = e;
				e1->m_scene = shared_from_this();

				auto& rb2d = e1->GetComponent<RigidBody2DComponent>();
				auto& transform = e1->GetComponent<TransformComponent>();

				b2Body* body = (b2Body*)rb2d.body;

				auto& position = body->GetPosition();
				transform.position.x = position.x;
				transform.position.y = position.y;
				//transform.rotation.z = body->GetRotation();
			}
		}

		// 3d physics
		{
			m_PhysicsSystem->step(ts);
			auto physicscomponentview = m_registry.view<PhysicsComponent>();

			for (auto e : physicscomponentview) {
				auto& pc = physicscomponentview.get<PhysicsComponent>(e);
				std::shared_ptr<Entity> entity = std::make_shared<Entity>();
				entity->id = e;
				entity->m_scene = shared_from_this();
				auto& tc = entity->GetComponent<TransformComponent>();

				glm::quat rot = PhysicsComponent::EulerToQuat(tc.rotation);
				pc.updateGameTransform(tc.position, rot);
				tc.rotation = PhysicsComponent::QuatToEuler(rot);
			}
		}

	}




	// LATER REMOVED
	// ! Now it fetches TransformComponent from entt registry and later it fetches spriterenderercomponenet it includes colors and textures
	// if cameraexist in Scene hierarchy panel then start rendering
	// else rendering processs doesnt't happen
	if (cameraexist) {

		// render skybox
		{
			auto e = GetPrimaryCameraEntity();
			glm::mat4 cameraproj = glm::mat4(1.0f);
			glm::mat4 view = glm::mat4(1.0f);

			auto& cc = e->GetComponent<CameraComponent>();
			auto& tc = e->GetComponent<TransformComponent>();
			if (cc.cameratype == Orthographic) {
				cameraproj = cc.camerao.viewprojection;
				view = glm::inverse(tc.getmatrix());
			}
			else {
				cameraproj = cc.camerap.viewprojection;
				view = glm::lookAt(tc.position, tc.position + cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
			}

			auto skyboxshader = ShaderManager::Get("skybox");
			skyboxshader->Bind();
			skyboxshader->SetMat4("view", view);
			skyboxshader->SetMat4("projection", cameraproj);
			app.skybox->Draw(*skyboxshader);

		}

		Renderer2D::UseShader(ShaderManager::Get("2d"));


		// Render 2D Sprites, Update Animations, Render Fonts
		{
			Renderer2D::BeginScene(cameramatrix);

			// Render 2D Sprites 
			{
				for (auto e : entities) {
					if (e->HasComponent<SpriteRendererComponent>() && e->GetComponent<NameComponent>().active) {
						auto& sc = e->GetComponent<SpriteRendererComponent>();
						auto transform = e->GetComponent<TransformComponent>();
						auto& nc = e->GetComponent<NameComponent>();
						if (sc.type == 0)
							Renderer2D::DrawRotatedQuad(transform.position, glm::vec2(transform.scale.x, transform.scale.y), transform.rotation.z, sc.color);
						else if (sc.type == 1) {
							transform.scale.x *= sc.m_subtexture.X_Multiplier;
							transform.scale.y *= sc.m_subtexture.Y_Multiplier;
							Renderer2D::DrawRotatedQuad(transform.position, glm::vec2(transform.scale.x, transform.scale.y), transform.rotation.z, sc.m_subtexture, sc.opacity);
						}

					}
				}
			}

			// animation manager
			AnimationManager::Update(ts);

			// Render Fonts
			{
				for (auto e : entities) {
					if (e->HasComponent<FontRendererComponent>()) {
						auto& fc = e->GetComponent<FontRendererComponent>();
						auto& transform = e->GetComponent<TransformComponent>();

						ui::fr->PrintString(fc.text, transform.position.x, transform.position.y, transform.position.z, fc.pixelsize, { 1.0f, 1.0f, 1.0f, fc.opacity });
					}

				}
			}

			Renderer2D::EndScene();

		}

		// draw shadowmaps
		{
			auto lights = m_registry.view<LightComponent, TransformComponent>();
			for (auto e1 : lights) {
				auto& lc = lights.get<LightComponent>(e1);
				auto& tc = lights.get<TransformComponent>(e1);
				lc.Update();

				lc.m_FrameBuffer.Bind();
				glViewport(0, 0, Window::Width, Window::Height);
				glClear(GL_DEPTH_BUFFER_BIT);

				glEnable(GL_DEPTH_TEST);
				glDepthMask(GL_TRUE);
				glDepthFunc(GL_LESS);

				glm::vec3 position = tc.position;
				glm::mat4 view = glm::lookAt(position, position + lc.direction, glm::vec3(0.0f, 1.0f, 0.0f));

				auto ashader = ShaderManager::Get("shadowmap");

				if (lc.lighttype == DIRECTIONAL_LIGHT) {

					// do rendering

					float w = (float)Window::Width / Window::Height;
					w /= 2;
					float h = 1.0f / 2;

					glm::mat4 proj = glm::ortho(-w, w, -h, h, 0.1f, farplane);

					glm::mat4 viewproj = proj * view;
					Render3DModels(ashader, viewproj);
				}

				else if (lc.lighttype == SPOT_LIGHT) {

					float w = (float)Window::Width / Window::Height;
					float h = 1.0f;

					glm::mat4 proj = glm::perspective(glm::radians(90.0f), w, 0.1f, farplane);

					glm::mat4 viewproj = proj * view;
					Render3DModels(ashader, viewproj);
				}

				else if (lc.lighttype == POINT_LIGHT) {
					glm::mat4 viewproj[6];

					auto shader = ShaderManager::Get("cubeshadowmap");

					float w = (float)Window::Width / Window::Height;
					float h = 1.0f;

					glm::mat4 proj = glm::perspective(glm::radians(90.0f), w, 0.1f, farplane);

					viewproj[0] = proj * glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
					viewproj[1] = proj * glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
					viewproj[2] = proj * glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
					viewproj[3] = proj * glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
					viewproj[4] = proj * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
					viewproj[5] = proj * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

					shader->Bind();
					for (unsigned int i = 0; i < 6; i++) {

						std::string varname = std::string("viewproj[") + std::to_string(i) + std::string("]");
						shader->SetMat4(varname.c_str(), viewproj[i]);
					}

					shader->SetFloat("farplane", farplane);
					shader->SetVec3("lightpos", position);

					Render3DModels(shader, glm::mat4(1.0f));
				}
			}
		}


		// render 3d models 
		{

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDepthFunc(GL_LEQUAL);


			auto mshader = ShaderManager::Get("3d");
			glm::mat4 viewproj = glm::mat4(1.0f);

			auto lights = m_registry.view<TransformComponent, LightComponent>();

			mshader->Bind();

			int samplers[16];

			unsigned int index = 0;

			for (auto e : lights) {
				auto& lc = lights.get<LightComponent>(e);
				auto& tc = lights.get<TransformComponent>(e);

				std::string name = std::string("lights[") + std::to_string(index) + std::string("]");
				std::string camname = std::string("cammatrix[") + std::to_string(index) + std::string("]");
				std::string indname = name + std::string(".index");
				std::string typname = name + std::string(".type");
				std::string colorname = name + std::string(".color");

				mshader->SetVec3(colorname.c_str(), lc.color);
				mshader->SetInt(indname.c_str(), index);
				mshader->SetInt(typname.c_str(), (int)lc.lighttype);

				if (lc.lighttype == DIRECTIONAL_LIGHT || lc.lighttype == SPOT_LIGHT) {
					std::string dirname = name + std::string(".direction");
					mshader->SetVec3(dirname.c_str(), lc.direction);
					lc.m_Texture->Bind(16 + index);
					std::string texturevar = std::string("textureshadowmap[") + std::to_string(index) + std::string("]");
					mshader->SetInt(texturevar.c_str(), 16 + index);
				}

				if (lc.lighttype == POINT_LIGHT || lc.lighttype == SPOT_LIGHT) {
					std::string posname = name + std::string(".position");
					mshader->SetVec3(posname.c_str(), tc.position);
				}

				if (lc.lighttype == SPOT_LIGHT) {
					std::string degreename = name + std::string(".degree");
					mshader->SetFloat(degreename.c_str(), lc.angle);
				}

				if (lc.lighttype == POINT_LIGHT) {
					lc.m_Texture->Bind(16 + index);
					std::cout << lc.m_Texture->m_TextureType << std::endl;
					std::string name = std::string("cubeshadowmap[") + std::to_string(index) + std::string("]");
					mshader->SetInt(name.c_str(), index);
				}


				// set viewprojection matrix
				if (lc.lighttype == DIRECTIONAL_LIGHT) {
					float w = (float)Window::Width / Window::Height;
					w /= 2;
					float h = 1.0f / 2;

					glm::mat4 proj = glm::ortho(-w, w, -h, h, 0.1f, farplane);

					glm::mat4 view = glm::lookAt(tc.position, tc.position + lc.direction, glm::vec3(0.0f, 1.0f, 0.0f));

					glm::mat4 viewproj = proj * view;

					mshader->SetMat4(camname.c_str(), viewproj);
				}

				else if (lc.lighttype == SPOT_LIGHT) {
					
					float w = (float)Window::Width / Window::Height;
					float h = 1.0f;

					glm::mat4 proj = glm::perspective(glm::radians(90.0f), w, 0.1f, farplane);
					glm::mat4 view = glm::lookAt(tc.position, tc.position + lc.direction, glm::vec3(0.0f, 1.0f, 0.0f));

					glm::mat4 viewproj = proj * view;

					mshader->SetMat4(camname.c_str(), viewproj);
				}

				index++;
			}

			std::string name = std::string("lights[") + std::to_string(index) + std::string("]");
			std::string indname = name + std::string(".index");
			mshader->SetInt(indname.c_str(), -1);

			auto cameraentity = GetPrimaryCameraEntity();
			auto& cc = cameraentity->GetComponent<CameraComponent>();
			auto& tc = cameraentity->GetComponent<TransformComponent>();

			if (cc.cameratype == CameraType::Orthographic) {
				viewproj = cc.camerao.viewprojection * glm::inverse(tc.getmatrix());
			}
			else {
				viewproj = cc.camerap.viewprojection * glm::lookAt(tc.position, tc.position + cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
			}


			Render3DModels(mshader, viewproj);

		}


		// for render gbuffer
		{
			app.gBuffer->Bind();
			glDisable(GL_BLEND);
			glDepthFunc(GL_LEQUAL);
			glViewport(0, 0, Window::Width, Window::Height);
			//glDisable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			auto mshader = ShaderManager::Get("gbuffer");
			glm::mat4 viewproj = glm::mat4(1.0f);

			mshader->Bind();

			auto cameraentity = GetPrimaryCameraEntity();
			auto& cc = cameraentity->GetComponent<CameraComponent>();
			auto& tc = cameraentity->GetComponent<TransformComponent>();

			if (cc.cameratype == CameraType::Orthographic) {
				viewproj = cc.camerao.viewprojection * glm::inverse(tc.getmatrix());
			}
			else {
				viewproj = cc.camerap.viewprojection * glm::lookAt(tc.position, tc.position + cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
			}


			Render3DModels(mshader, viewproj);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		}

		if (!app.EditorMode) {
			// renders particle first 
						// this renders particles
			Renderer2D::UseShader(ShaderManager::Get("2dparticle"));
			for (auto e : entities) {
				if (e->HasComponent<ParticleGeneratorComponent>()) {
					auto& pg = e->GetComponent<ParticleGeneratorComponent>();
					if (pg.active) {
						auto& transform = e->GetComponent<TransformComponent>();
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

		 
		// for render 2d box colliders
		Renderer2D::UseShader(ShaderManager::Get("2d"));


		if (app.EditorMode && EditorLayer::showcolliders) {
			auto view = m_registry.view<BoxCollider2DComponent>();
			Renderer2D::BeginScene(cameramatrix);
			for (auto e : view) {
				std::shared_ptr<Entity> et = std::make_shared<Entity>();
				et->id = e;
				et->m_scene = shared_from_this();
				if (et->HasComponent<RigidBody2DComponent>()) {
					auto& transform = et->GetComponent<TransformComponent>();
					auto& bc = et->GetComponent<BoxCollider2DComponent>();
					auto& rc = et->GetComponent<RigidBody2DComponent>();

					if (bc.sameasscale)
						Renderer2D::DrawRotatedQuad(glm::vec3(transform.position.x, transform.position.y, 0.01f), { transform.scale.x, transform.scale.y }, transform.rotation.z, glm::vec4(1.0f));
					else
						Renderer2D::DrawRotatedQuad(glm::vec3(transform.position.x, transform.position.y, 0.01f), { bc.width, bc.height }, transform.rotation.z, glm::vec4(1.0f));

				}
			}
			Renderer2D::EndScene();
		}


		// for render 3d physics colliders 
		// currently it renders only cube colliders
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		if (app.EditorMode && EditorLayer::showcolliders) {
			auto view = m_registry.view<PhysicsComponent>();

			auto ce = GetPrimaryCameraEntity();
			auto& cc = ce->GetComponent<CameraComponent>();
			auto& tc = ce->GetComponent<TransformComponent>();

			auto shader = ShaderManager::Get("3dline");
			shader->Bind();

			glm::mat4 viewproj = glm::mat4(1.0f);

			if (cc.cameratype == CameraType::Orthographic) {
				viewproj = cc.camerao.viewprojection * glm::inverse(tc.getmatrix());
			}
			else {
				viewproj = cc.camerap.viewprojection * glm::lookAt(tc.position, tc.position + cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
			}

			for (auto e : view) {


				std::shared_ptr<Entity> ent = std::make_shared<Entity>();
				ent->id = e;
				ent->m_scene = shared_from_this();

				auto& pc = ent->GetComponent<PhysicsComponent>();
				auto tc = ent->GetComponent<TransformComponent>();

				tc.scale = pc.BoxHalfExtents * 2.0f;

				cc.camerap.Update();
				

				shader->SetMat4("viewproj", viewproj);

				if (pc.ShapeType == PhysicsShapeType::Box) {
					shader->SetMat4("model", tc.worldtransform* tc.getmatrix());
					std::shared_ptr<Model> cubemodel = ModelManager::GetModel("res/cube.obj");
					cubemodel->Draw(*shader);
				}

				else if (pc.ShapeType == PhysicsShapeType::Sphere) {
					tc.scale = glm::vec3(pc.SphereRadius);
					shader->SetMat4("model", tc.worldtransform* tc.getmatrix());
					std::shared_ptr<Model> spheremodel = ModelManager::GetModel("res/sphere.obj");
					spheremodel->Draw(*shader);
				}
			}
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//Renderer2D::UseShader(ShaderManager::Get("2d"));

		// if entity has particle generator then render particle generator logo
		if (app.EditorMode) {
			auto view = m_registry.view<ParticleGeneratorComponent>();
			Renderer2D::BeginScene(cameramatrix);
			for (auto e : view) {
				std::shared_ptr<Entity> et = std::make_shared<Entity>();
				et->id = e;
				et->m_scene = shared_from_this();
				auto& transform = et->GetComponent<TransformComponent>();
				Renderer2D::DrawQuad(transform.position, { 0.3f, 0.3f }, EditorLayer::particlegeneratorsubtexture);
			}
			Renderer2D::EndScene();
		}




		// show chainshape colliders
		if (app.EditorMode) {

			if (EditorLayer::camera_locked) {
				Renderer2D::BeginScene(glm::ortho(-Window::Ratio, Window::Ratio, -1.0f, 1.0f));
				ui::fr->PrintString("Locked Camera", 0.0f, 0.5f, 64.0f);
				Renderer2D::EndScene();
			}

			// for showing messages
			float msgbatch_y = -0.3f;
			Renderer2D::BeginScene(glm::ortho(-Window::Ratio, Window::Ratio, -1.0f, 1.0f));
			for (auto& msg : s_msgbatch.messages) {

				float opacity = 1.0f;

				if (msg.timer < msg.fadetime) {
					float diff = msg.fadetime - msg.timer;
					opacity = 1.0f - (diff / msg.fadetime);
				}
				else if (msg.timer > msg.lifetime - msg.fadetime) {
					float diff = msg.lifetime - msg.timer;
					opacity = diff / msg.fadetime;
				}

				glm::vec4 color = glm::vec4(1.0f);
				color.a = opacity;

				if (msg.type == MessageType::error)
					color = glm::vec4(1.0f, 0.0f, 0.0f, opacity);
				else if (msg.type == MessageType::warning)
					color = glm::vec4(0.0f, 1.0f, 1.0f, opacity);

				ui::fr->PrintString(msg.text, 0.0f, msgbatch_y, 64.0f, color);
				msgbatch_y -= 0.1f;
			}
			Renderer2D::EndScene();

		}

		
		//s_MainSetting.Render();
	}

}


void Scene::Render3DModels(std::shared_ptr<Shader> &shader, glm::mat4 viewproj) {
	for (auto e : entities) {
		if (e->HasComponent<Model3DComponent>()) {
			auto& mc = e->GetComponent<Model3DComponent>();
			auto& tc = e->GetComponent<TransformComponent>();

			if (mc.mModel != nullptr) {
				auto e = GetPrimaryCameraEntity();
				auto& cc = e->GetComponent<CameraComponent>();

				shader->Bind();
				shader->SetMat4("viewproj", viewproj);
				shader->SetMat4("model", tc.worldtransform * tc.getmatrix());
				mc.mModel->Draw(*shader);
			}
		}
	}
}