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
#include <FontManager.h>

#include <ShaderManager.h>

#include <iostream>

#include <ParticleSystem.h>

#define HORIZONTAL_BLUR 0
#define VERTICAL_BLUR 1

float shadoworthosize = 20.0f;

float farplane = 100.0f;

// for create Entity
// this function register Entity in entt registry and return Entity structure

MainSetting Scene::s_MainSetting;

MessageBatch Scene::s_msgbatch;

ParticleProps tpp = {};

ParticleSystem tps;

bool lightupdate = false;

int counter = 0;




float lastmousex = Window::Width / 2;
float lastmousey = Window::Height / 2;


glm::vec3 Scene::cameraorientation = glm::vec3(1.0f);



#define AppScene Application::GetInstance().s_serializer.m_scene


Scene::Scene() {
	gvbo = VBO(sizeof(vertices), vertices, GL_STATIC_DRAW);

	gvao = VAO();
	gvao.LinkVBO(gvbo, 0, 3, 5 * sizeof(float), 0);
	gvao.LinkVBO(gvbo, 1, 2, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	gebo = EBO(sizeof(indices), indices);
}

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

	std::cout << (unsigned int)a_Entity->id << std::endl;
	
	// remove entity shared ptr from parents entities child lists
	auto& rc = a_Entity->GetComponent<RelationshipComponent>();

	if (rc.parentEntity) {
		if (rc.parentEntity->HasComponent<RelationshipComponent>())
			std::cout << "parent have " << std::endl;
		auto& prc = rc.parentEntity->GetComponent<RelationshipComponent>();
		
		int index = -1; 

		unsigned int len = prc.childEntities.size();
		for (unsigned int i = 0; i < len; i++) {
			
			if (prc.childEntities[i]->id == a_Entity->id)
				index = i;
		}

		if (index >= 0)
			prc.childEntities.erase(prc.childEntities.begin() + index);

	}

	// Also remove this entity from its children's parent pointers
	for (auto child : rc.childEntities) {
		auto& crc = child->GetComponent<RelationshipComponent>();
		crc.parentEntity = nullptr;
		DestroyEntity(child);
	}
	rc.childEntities.clear();


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
	tpp.velocity = { 0.0f, 0.0f, 0.0f };
	tpp.velocityvariation = { 0.1f, 0.5f, 0.1f };
	tpp.colorbegin = glm::vec4(1.0f);
	tpp.colorend = glm::vec4(glm::vec3(0.0f), 1.0f);
	tpp.sizebegin = 1.0f;
	tpp.sizeend = 0.2f;


	if (m_physicsType == BOX2D) {
		physicsworld = new b2World({ 0.0f, -9.0f });


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
	}


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



	if (m_physicsType == BULLET) {
		// later removed
		m_PhysicsSystem = new PhysicsSystem();

		// create 3d physics components
		auto physicscomponentview = m_registry.view<PhysicsComponent>();

		for (auto e : physicscomponentview) {
			auto& pc = physicscomponentview.get<PhysicsComponent>(e);
			std::shared_ptr<Entity> entity = std::make_shared<Entity>();
			entity->id = e;
			entity->m_scene = shared_from_this();
			auto& tc = entity->GetComponent<TransformComponent>();
			auto& rc = entity->GetComponent<RelationshipComponent>();
			auto& nc = entity->GetComponent<NameComponent>();

			pc.InitialPosition = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			pc.InitialRotation = PhysicsComponent::EulerToQuat(tc.rotation);

			if (pc.OnlyPositive)
			{
				if (pc.ShapeType == PhysicsShapeType::Box)
					pc.InitialPosition.y += pc.BoxHalfExtents.y;
				else if (pc.ShapeType == PhysicsShapeType::Sphere)
					pc.InitialPosition.y += pc.SphereRadius;
			}
			pc.initializeRuntimeBody(m_PhysicsSystem->getWorld());

			glm::quat rot = PhysicsComponent::EulerToQuat(tc.rotation);
			pc.updateGameTransform(tc.position, rot);
			tc.SetRotationQuat(rot);
			//tc.rotation = PhysicsComponent::QuatToEuler(rot);

			if (rc.parentEntity) {
				auto& ptc = rc.parentEntity->GetComponent<TransformComponent>();
				glm::mat4 parentWorldInv = glm::inverse(ptc.worldtransform);
				glm::vec3 worldPos = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				tc.position = glm::vec3(parentWorldInv * glm::vec4(worldPos, 1.0f));
			}
		}
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

	if (m_physicsType == BOX2D) {
		if (physicsworld) {
			delete physicsworld;
			physicsworld = nullptr;
		}
	}

	auto view = m_registry.view<ScriptComponent>();

	for (auto e : view) {
		auto& sc = view.get<ScriptComponent>(e);
		sc.destroy();
	}

	if (m_physicsType == BULLET) {

		if (m_PhysicsSystem) {
			// for bullet physics
			auto physicscomponentview = m_registry.view<PhysicsComponent>();

			for (auto e : physicscomponentview) {
				auto& pc = physicscomponentview.get<PhysicsComponent>(e);
				pc.destroyRuntimeBody(m_PhysicsSystem->getWorld());
			}

			delete m_PhysicsSystem;
			m_PhysicsSystem = nullptr;
			
		}
	}
}

bool Scene::OnEvent(Event& e) {

	if (e.is(Event_Type::Window_Resized)) {
		auto view = m_registry.view<CameraComponent>();
		for (auto e1 : view) {
			auto& cc = view.get<CameraComponent>(e1);
			cc.camerao.Update();
		}
		return true;
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

	glm::mat4 wt = transform * tc.getmatrix();

	tc.worldtransform = wt;

	for (auto e : rc.childEntities) {
		if (m_registry.valid(e->id)) {
			UpdateGlobalTransform(e, wt);
		}
	}
}

void Scene::UpdateLightStatus(const glm::vec3& position) {

	auto view = m_registry.view<TransformComponent, LightComponent>();

	for (auto e : view) {
		glm::vec3 lightpos = view.get<TransformComponent>(e).position;
		auto& lc = view.get<LightComponent>(e);
		lc.EntityUpdate(lightpos, position);
	}
}

// this function Scene hierarchy entities
// and also do rendering of Scene hierarchy entities
void Scene::Update(float ts) {

	if (counter > 5)
		counter = 0;
	else
		counter++;

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

				glm::vec3 rotation = transform.rotation;

				ParticleSystem::yrotation = rotation.y;

				glm::vec3 orientation = glm::vec3(0.0f);

				orientation.x = -glm::sin(glm::radians(rotation.y)) * glm::cos(glm::radians(rotation.x));
				orientation.z = glm::cos(glm::radians(rotation.y + 180.0f)) * glm::cos(glm::radians(rotation.x));
				orientation.y = glm::sin(glm::radians(rotation.x));

				cameraorientation = orientation;

				bool renderdepthandssao = false;

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

					if (Input::KeyPressed(KEY_W)) {
						transform.position += orientation * EditorLayer::EditorCameraSpeed;
						renderdepthandssao = true;
					}
					if (Input::KeyPressed(KEY_S)) {
						transform.position -= orientation * EditorLayer::EditorCameraSpeed;
						renderdepthandssao = true;
					}
					if (Input::KeyPressed(KEY_A)) {
						glm::vec3 a = glm::cross(orientation, glm::vec3(0.0f, 1.0f, 0.0f)) * EditorLayer::EditorCameraSpeed;
						transform.position -= a;
						renderdepthandssao = true;
					}
					if (Input::KeyPressed(KEY_D)) {
						glm::vec3 a = glm::cross(orientation, glm::vec3(0.0f, 1.0f, 0.0f)) * EditorLayer::EditorCameraSpeed;
						transform.position += a;
						renderdepthandssao = true;
					}
				}

				// when we develop 3d games
				if (app.EditorMode && !EditorLayer::camera_locked && EditorLayer::s_EditorCameraType == THREE_AXIS_MOVE_CAMERA) {

					if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_RIGHT)) {
						double x, y;
						glfwGetCursorPos(Window::ID, &x, &y);

						float offsetx = x - lastmousex;
						float offsety = y - lastmousey;

						glm::vec3 newrot = transform.rotation;

						newrot.x += glm::degrees(offsety / Window::Height);
						newrot.y += glm::degrees(offsetx / Window::Width);

						transform.SetRotationEuler(newrot);

						if (offsetx != 0.0f || offsety != 0.0f) {
							renderdepthandssao = true;
						}

						lastmousex = x;
						lastmousey = y;
					}
				}
				
				if (renderdepthandssao)
					Application::captureshadowmap = true;


				if (cc.cameratype == CameraType::Orthographic) {
					cameramatrix = cc.camerao.viewprojection * glm::inverse(transform.worldtransform);
				}
				else {
					cc.camerap.Update();
					glm::vec3 cameraworldpos = glm::vec3(transform.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
					cameramatrix = cc.camerap.viewprojection * glm::lookAt(cameraworldpos, cameraworldpos + cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
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
			auto& app = Application::GetInstance();
			// for running
			for (auto e : group1) {
				auto& scc = group1.get<ScriptComponent>(e);

				if (scc.script != nullptr && app.s_serializer.m_scene->m_registry.valid(e))
					scc.script->Update(ts);
			}

		}

		// physics
		{
			if (m_physicsType == BOX2D) {
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
		}

		// 3d physics
        {
			if (m_physicsType == BULLET) {
				m_PhysicsSystem->step(ts);
				auto physicscomponentview = m_registry.view<PhysicsComponent>();

				for (auto e : physicscomponentview) {
					auto& pc = physicscomponentview.get<PhysicsComponent>(e);
					std::shared_ptr<Entity> entity = std::make_shared<Entity>();
					entity->id = e;
					entity->m_scene = shared_from_this();
					auto& tc = entity->GetComponent<TransformComponent>();
					auto& rc = entity->GetComponent<RelationshipComponent>();

					glm::quat rot = PhysicsComponent::EulerToQuat(tc.rotation);
					pc.updateGameTransform(tc.position, rot);
					// sync back: update both Euler and quaternion from physics quaternion
					tc.SetRotationQuat(rot);
					// position sync below (unchanged)
					if (rc.parentEntity) {
						auto& ptc = rc.parentEntity->GetComponent<TransformComponent>();
						glm::mat4 parentWorldInv = glm::inverse(ptc.worldtransform);
						tc.position = glm::vec3(parentWorldInv * glm::vec4(tc.position, 1.0f));
					}
				}
			}
        }

	}

	
	auto cameraent = GetPrimaryCameraEntity();

	auto gbufferShader = ShaderManager::Get("gbuffer");
	auto ssaoShader = ShaderManager::Get("ssao");
	auto lightingShader = ShaderManager::Get("3dfromgbuffer");
	auto skyboxshader = ShaderManager::Get("skybox");
	auto ashader = ShaderManager::Get("shadowmap");
	auto cubeshadowmapshader = ShaderManager::Get("cubeshadowmap");
	auto computeblur = ShaderManager::Get("computeblur");
	auto downsampleshader = ShaderManager::Get("downsample");
	auto rgbablur = ShaderManager::Get("rgbablur");
	auto upsample = ShaderManager::Get("upsample");
	auto linearblur = ShaderManager::Get("linearblur");
	auto finalbloom = ShaderManager::Get("finalbloom");

	// LATER REMOVED
	// ! Now it fetches TransformComponent from entt registry and later it fetches spriterenderercomponenet it includes colors and textures
	// if cameraexist in Scene hierarchy panel then start rendering
	// else rendering processs doesnt't happen
	if (cameraexist) {

		app.finalFrameBuffer->Bind();

		// render skybox
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glm::mat4 cameraproj = glm::mat4(1.0f);
			glm::mat4 view = glm::mat4(1.0f);

			auto& cc = cameraent->GetComponent<CameraComponent>();
			auto& tc = cameraent->GetComponent<TransformComponent>();

			glm::vec3 camerapos = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

			if (cc.cameratype == Orthographic) {
				cc.camerao.Update();
				cameraproj = cc.camerao.viewprojection;
				view = glm::inverse(tc.getmatrix());
			}
			else {
				cameraproj = cc.camerap.viewprojection;
				view = glm::lookAt(camerapos, camerapos + cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
			}

			skyboxshader->Bind();
			skyboxshader->SetMat4("view", view);
			skyboxshader->SetMat4("projection", cameraproj);
			app.skybox->Draw(*skyboxshader);

		}

		char strid[128];


		// draw shadowmaps
		if(true)
		{
		
			auto lights = m_registry.view<LightComponent, TransformComponent>();
			for (auto e1 : lights) {
				auto& lc = lights.get<LightComponent>(e1);
				auto& tc = lights.get<TransformComponent>(e1);
				lc.Update();

				if (lc.rerender) {

					lc.m_FrameBuffer.Bind();
					glViewport(0, 0, Window::Width, Window::Height);
					glClear(GL_DEPTH_BUFFER_BIT);

					glEnable(GL_DEPTH_TEST);
					glDepthMask(GL_TRUE);
					glDepthFunc(GL_LESS);

					glm::vec3 position = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
					glm::mat4 view = glm::lookAt(position, position + lc.direction, glm::vec3(0.0f, 1.0f, 0.0f));

					if (lc.lighttype == DIRECTIONAL_LIGHT) {

						// do rendering

						float w = (float)Window::Width / Window::Height;

						glm::mat4 proj = glm::ortho(-shadoworthosize * w, shadoworthosize * w, -shadoworthosize, shadoworthosize, 0.1f, farplane);

						glm::mat4 viewproj = proj * glm::lookAt(-lc.direction * farplane, -lc.direction * farplane + lc.direction, glm::vec3(0.0f, 1.0f, 0.0f));
						Render3DModels(ashader, viewproj);
					}

					else if (lc.lighttype == SPOT_LIGHT) {

						float w = (float)Window::Width / Window::Height;
						float h = 1.0f;

						glm::mat4 proj = glm::perspective(glm::radians((lc.angle + 10.0f) * 2.0f), 1.0f, 1.0f, farplane);

						glm::mat4 viewproj = proj * view;
						Render3DModels(ashader, viewproj);
					}

					else if (lc.lighttype == POINT_LIGHT) {
						glViewport(0, 0, Window::Width, Window::Width);
						glClear(GL_DEPTH_BUFFER_BIT);
						glm::mat4 viewproj[6];

						float w = (float)Window::Width / Window::Height;
						float h = 1.0f;

						glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, farplane);

						viewproj[0] = proj * glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
						viewproj[1] = proj * glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
						viewproj[2] = proj * glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
						viewproj[3] = proj * glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
						viewproj[4] = proj * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
						viewproj[5] = proj * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

						cubeshadowmapshader->Bind();
						for (unsigned int i = 0; i < 6; i++) {
							sprintf(strid, "viewproja[%d]", i);
							cubeshadowmapshader->SetMat4(strid, viewproj[i]);
						}

						cubeshadowmapshader->SetFloat("farplane", farplane);
						cubeshadowmapshader->SetVec3("lightpos", position);

						Render3DModels(cubeshadowmapshader, glm::mat4(1.0f));
					}

					lc.rerender = false;
				}
			}
		}

		

		// for render gbuffer
		{
			app.gBuffer->Bind();
			glDisable(GL_BLEND);
			glDepthFunc(GL_LESS);
			glViewport(0, 0, Window::Width, Window::Height);
			//glDisable(GL_DEPTH_TEST);
			glEnable(GL_STENCIL_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			glm::mat4 viewproj = glm::mat4(1.0f);

			gbufferShader->Bind();


			auto& cc = cameraent->GetComponent<CameraComponent>();
			auto& tc = cameraent->GetComponent<TransformComponent>();

			if (cc.cameratype == CameraType::Orthographic) {
				viewproj = cc.camerao.viewprojection * glm::inverse(tc.worldtransform);
			}
			else {
				glm::vec3 pos = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				viewproj = cc.camerap.viewprojection * glm::lookAt(pos, pos + cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
			}


			Render3DModels(gbufferShader, viewproj, ts);

			auto outlineshader = ShaderManager::Get("outline");

			outlineshader->Bind();

			Render3DModels(outlineshader, viewproj, ts, true);


			// blur bloom
			if(true)
			{
				// downsample
				app.downsampledbloomF->Bind();
				glViewport(0, 0, Window::Width / 2, Window::Height / 2);

				downsampleshader->Bind();
				app.bloomBuffer->Bind(0);


				gvao.Bind();
				gebo.Bind();
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

				// horizontal blur
				app.bloomhorizontalF->Bind();
				rgbablur->Bind();
				app.downsampledbloomBuffer->Bind(0);
				rgbablur->SetInt("blurtype", HORIZONTAL_BLUR);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

				// vertical blur
				app.bloomverticalF->Bind();
				rgbablur->Bind();
				app.bloomhorizontalblurred->Bind(0);
				rgbablur->SetInt("blurtype", VERTICAL_BLUR);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


				app.finalbloomF->Bind();

				glViewport(0, 0, Window::Width, Window::Height);

				finalbloom->Bind();
				app.bloomBuffer->Bind(0);
				app.bloomverticalblurred->Bind(1);

				finalbloom->SetInt("i1", 0);
				finalbloom->SetInt("i2", 1);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			}


		}


		// render ssAO BUFFER
		if(true)
		{
			glViewport(0, 0, Window::Width / 4, Window::Height / 4);
			app.ssaoFBuffer->Bind();
			//glDisable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT);


			glm::mat4 viewproj = glm::mat4(1.0f);

			ssaoShader->Bind();

			auto& cc = cameraent->GetComponent<CameraComponent>();
			auto& tc = cameraent->GetComponent<TransformComponent>();

			if (cc.cameratype == CameraType::Orthographic) {
				viewproj = cc.camerao.viewprojection * glm::inverse(tc.worldtransform);
			}
			else {
				glm::vec3 pos = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				viewproj = cc.camerap.viewprojection * glm::lookAt(pos, pos + cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
			}

			app.PositionBuffer->Bind(0);
			app.NormalBuffer->Bind(1);
			app.DepthBuffer->Bind(2);

			ssaoShader->SetInt("positionbuffer", 0);
			ssaoShader->SetInt("normalbuffer", 1);
			ssaoShader->SetInt("depthbuffer", 2);

			ssaoShader->SetMat4("viewproj", viewproj);

			ssaoShader->SetFloat("camerarange", (cc.camerap.far - cc.camerap.near));

			gvao.Bind();
			gebo.Bind();
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		
			// blur ssao buffer


			linearblur->Bind();

			linearblur->SetInt("blurtype",VERTICAL_BLUR);

			app.ssaoblurFBuffer->Bind();

			app.ssaobuffer->Bind(0);
			
			gvao.Bind();
			gebo.Bind();

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			computeblur->SetInt("blurtype", HORIZONTAL_BLUR);

			app.ssaoblurFBuffer2->Bind();
			app.blurredssaobuffer->Bind(0);
			
			gvao.Bind();
			gebo.Bind();

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			Application::captureshadowmap = false;

		}

		 // 3d rendering from gbuffer
        {
			glViewport(0, 0, Window::Width, Window::Height);
            app.finalFrameBuffer->Bind();
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);

            glm::mat4 viewproj = glm::mat4(1.0f);

            auto lights = m_registry.view<TransformComponent, LightComponent>();

            lightingShader->Bind();

            unsigned int index = 0;
            char lightname[256]; // pre-allocate buffer for string building

            for (auto e : lights) {
                auto& lc = lights.get<LightComponent>(e);
                auto& tc = lights.get<TransformComponent>(e);

				if (lc.active) {

					glm::vec3 position = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

					// Use snprintf instead of std::string concatenation (10x faster)
					snprintf(lightname, sizeof(lightname), "lights[%u].color", index);
					lightingShader->SetVec3(lightname, lc.color);

					snprintf(lightname, sizeof(lightname), "lights[%u].index", index);
					lightingShader->SetInt(lightname, index);

					snprintf(lightname, sizeof(lightname), "lights[%u].type", index);
					lightingShader->SetInt(lightname, (int)lc.lighttype);

					if (lc.lighttype == DIRECTIONAL_LIGHT || lc.lighttype == SPOT_LIGHT) {
						snprintf(lightname, sizeof(lightname), "lights[%u].direction", index);
						lightingShader->SetVec3(lightname, lc.direction);
						lc.m_Texture->Bind(16 + index);
						snprintf(lightname, sizeof(lightname), "textureshadowmap[%u]", index);
						lightingShader->SetInt(lightname, 16 + index);
					}

					snprintf(lightname, sizeof(lightname), "lights[%u].position", index);
					lightingShader->SetVec3(lightname, position);

					if (lc.lighttype == SPOT_LIGHT) {
						snprintf(lightname, sizeof(lightname), "lights[%u].degree", index);
						lightingShader->SetFloat(lightname, lc.angle);
					}

					if (lc.lighttype == POINT_LIGHT) {
						lc.m_Texture->Bind(16 + index);
						snprintf(lightname, sizeof(lightname), "cubeshadowmap[%u]", index);
						lightingShader->SetInt(lightname, 16 + index);
					}

					// set viewprojection matrix
					if (lc.lighttype == DIRECTIONAL_LIGHT) {
						float w = (float)Window::Width / Window::Height;
						glm::mat4 proj = glm::ortho(-shadoworthosize * w, shadoworthosize * w, -shadoworthosize, shadoworthosize, 0.1f, farplane);
						glm::mat4 view = glm::lookAt(-lc.direction * farplane, -lc.direction * farplane + lc.direction, glm::vec3(0.0f, 1.0f, 0.0f));
						snprintf(lightname, sizeof(lightname), "cammatrix[%u]", index);
						lightingShader->SetMat4(lightname, proj * view);
					}
					else if (lc.lighttype == SPOT_LIGHT) {
						glm::mat4 proj = glm::perspective(glm::radians((lc.angle + 10.0f) * 2.0f), 1.0f, 1.0f, farplane);
						glm::mat4 view = glm::lookAt(position, position + lc.direction, glm::vec3(0.0f, 1.0f, 0.0f));
						snprintf(lightname, sizeof(lightname), "cammatrix[%u]", index);
						lightingShader->SetMat4(lightname, proj * view);
					}

					index++;
				}
            }

            snprintf(lightname, sizeof(lightname), "lights[%u].index", index);
            lightingShader->SetInt(lightname, -1);

            auto cameraentity = GetPrimaryCameraEntity();
            auto& cc = cameraentity->GetComponent<CameraComponent>();
            auto& tc = cameraentity->GetComponent<TransformComponent>();

            glm::vec3 camerapos = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));	

            if (cc.cameratype == CameraType::Orthographic) {
                viewproj = cc.camerao.viewprojection * glm::inverse(tc.worldtransform);
            }
            else {
                viewproj = cc.camerap.viewprojection * glm::lookAt(camerapos, camerapos + cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
            }

            lightingShader->SetVec3("camerapos", camerapos);
            lightingShader->SetMat4("viewproj", viewproj);
            lightingShader->SetFloat("ambscale", ambientscale);

            app.PositionBuffer->Bind(0);
            app.NormalBuffer->Bind(1);
            app.AlbedoBuffer->Bind(2);
            app.DepthBuffer->Bind(3);
            app.blurredssaobuffer2->Bind(4);

            lightingShader->SetInt("positionbuffer", 0);
            lightingShader->SetInt("normalbuffer", 1);
            lightingShader->SetInt("albedobuffer", 2);
            lightingShader->SetInt("depthbuffer", 3);
            lightingShader->SetInt("ssaobuffer", 4);

            gvao.Bind();
            gebo.Bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // ... rest of bloom code ...
        

			
			
		}

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

						glm::vec3 position = glm::vec3(transform.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
						if (sc.type == 0)
							Renderer2D::DrawRotatedQuad(position, glm::vec2(transform.scale.x, transform.scale.y), transform.rotation, sc.color);
						else if (sc.type == 1) {
							transform.scale.x *= sc.m_subtexture.X_Multiplier;
							transform.scale.y *= sc.m_subtexture.Y_Multiplier;
							Renderer2D::DrawRotatedQuad(position, glm::vec2(transform.scale.x, transform.scale.y), transform.rotation, sc.m_subtexture, sc.opacity);
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

						glm::vec3 position = glm::vec3(transform.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

						if (fc.pixelsize < 8.0f)
							fc.pixelsize = 8.0f;

						auto fr = FontManager::GetFont(fc.pixelsize, "res/fonts/times.ttf");
						fr->PrintString(fc.text.c_str(), position.x, position.y, position.z, fc.pixelsize, {1.0f, 1.0f, 1.0f, fc.opacity});
					}

				}
			}

			Renderer2D::EndScene();

		}

		if (true) {
			// renders particle first 
						// this renders particles
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
			Renderer2D::UseShader(ShaderManager::Get("2dparticle"));
			for (auto e : entities) {
				if (e->HasComponent<ParticleGeneratorComponent>()) {
					auto& pg = e->GetComponent<ParticleGeneratorComponent>();
					if (pg.active) {
						auto& transform = e->GetComponent<TransformComponent>();
						glm::vec3 position = glm::vec3(transform.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
						pg.time += ts;
						if (pg.time > 1.0f / pg.count) {
							pg.m_particleprops.position = position;
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

			glDepthMask(GL_TRUE);
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


		// if entity has particle generator then render particle generator logo
		if (app.EditorMode) {
			auto view = m_registry.view<ParticleGeneratorComponent>();
			Renderer2D::BeginScene(cameramatrix);
			for (auto e : view) {
				std::shared_ptr<Entity> et = std::make_shared<Entity>();
				et->id = e;
				et->m_scene = shared_from_this();
				auto& transform = et->GetComponent<TransformComponent>();
				glm::vec3 wp = glm::vec3(transform.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				Renderer2D::DrawQuad(wp, { 0.3f, 0.3f }, EditorLayer::particlegeneratorsubtexture);
			}
			Renderer2D::EndScene();
		}


		// render ui
		{
			glm::mat4 orthomat = glm::ortho(-Window::Ratio, Window::Ratio, -1.0f, 1.0f);
			Renderer2D::BeginScene(orthomat);

			static char buffer[128];
			
			
			//ui::fr->PrintStringui(buffer, -1.6f, 0.7f, 0.0f, 64.0f);

			// render spriterenderercomponent
			auto spriteview = m_registry.view<UISpriteRendererComponent, TransformComponent>();
			for (auto e : spriteview) {
				auto& tc = spriteview.get<TransformComponent>(e);
				auto& sc = spriteview.get<UISpriteRendererComponent>(e);


				glm::vec3 position = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				if (sc.type == 0)
					Renderer2D::DrawRotatedQuad(position, glm::vec2(tc.scale.x, tc.scale.y), tc.rotation, sc.color);
				else if (sc.type == 1) {
					tc.scale.x *= sc.m_subtexture.X_Multiplier;
					tc.scale.y *= sc.m_subtexture.Y_Multiplier;
					Renderer2D::DrawRotatedQuad(position, glm::vec2(tc.scale.x, tc.scale.y), tc.rotation, sc.m_subtexture, sc.opacity);
				}
			}


			// render fontrenderercomponent
			auto fontview = m_registry.view<UIFontRendererComponent, TransformComponent>();
			for (auto e : fontview) {
				auto& tc = fontview.get<TransformComponent>(e);
				auto& fc = fontview.get<UIFontRendererComponent>(e);

				glm::vec3 position = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

				if (fc.pixelsize < 8.0f)
					fc.pixelsize = 8.0f;

				auto fr = FontManager::GetFont(fc.pixelsize, "res/fonts/times.ttf");
				fr->PrintString(fc.text.c_str(), position.x, position.y, position.z, fc.pixelsize, {1.0f, 1.0f, 1.0f, fc.opacity});
			}

			Renderer2D::EndScene();
		}



		// show chainshape colliders
		if (app.EditorMode) {

			auto fr = FontManager::GetFont(64, "res/fonts/times.ttf");
			Renderer2D::BeginScene(glm::ortho(-Window::Ratio, Window::Ratio, -1.0f, 1.0f));

			if (EditorLayer::camera_locked) {
				fr->PrintString("Locked Camera", 0.0f, 0.5f, 64.0f);
			}

			// for showing messages
			float msgbatch_y = -0.3f;
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

				fr->PrintString(msg.text.c_str(), 0.0f, msgbatch_y, 64.0f, color);
				msgbatch_y -= 0.1f;
			}
			Renderer2D::EndScene();

		}


		// for render 3d physics colliders 
		// currently it renders only cube colliders
		
		glEnable(GL_DEPTH_TEST);
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
                viewproj = cc.camerao.viewprojection * glm::inverse(tc.worldtransform);
            }
            else {
                glm::vec3 pos = glm::vec3(tc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                viewproj = cc.camerap.viewprojection * glm::lookAt(pos, pos + cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
            }

            // Cache models
            static std::shared_ptr<Model> cubeModel = ModelManager::GetModel("res/cube.obj");
            static std::shared_ptr<Model> sphereModel = ModelManager::GetModel("res/sphere.obj");
            static std::shared_ptr<Model> cylinderModel = ModelManager::GetModel("res/cylinder.obj");

            for (auto e : view) {
                std::shared_ptr<Entity> ent = std::make_shared<Entity>();
                ent->id = e;
                ent->m_scene = shared_from_this();

				auto& nc = ent->GetComponent<NameComponent>();

                auto& pc = ent->GetComponent<PhysicsComponent>();
                auto original_tc = ent->GetComponent<TransformComponent>();

				auto parententity = ent->GetComponent<RelationshipComponent>().parentEntity;
				glm::mat4 parentworld = glm::mat4(1.0f);
				if (parententity) {
					parentworld = parententity->GetComponent<TransformComponent>().worldtransform;
				}
				
                
                TransformComponent temp = original_tc;

                if (pc.OnlyPositive) {
                    if (pc.ShapeType == PhysicsShapeType::Box)
                        temp.position.y += pc.BoxHalfExtents.y;
                    else if (pc.ShapeType == PhysicsShapeType::Sphere)
                        temp.position.y += pc.SphereRadius;
                    else if (pc.ShapeType == PhysicsShapeType::Capsule)
                        temp.position.y += pc.BoxHalfExtents.y + pc.SphereRadius;
                }

                if (pc.ShapeType == PhysicsShapeType::Capsule) {
                    temp.scale.y = pc.BoxHalfExtents.y * 2.0f;
                    temp.scale.x = temp.scale.z = 1.0f;
                }
                else
                    temp.scale = pc.BoxHalfExtents * 2.0f;

                cc.camerap.Update();
                shader->SetMat4("viewproj", viewproj);

				glm::mat4 model = parentworld * temp.getmatrix();

                shader->SetMat4("model", model);

                if (pc.ShapeType == PhysicsShapeType::Box) {
                    cubeModel->Draw(*shader);
                }
                else if (pc.ShapeType == PhysicsShapeType::Sphere) {
                    sphereModel->Draw(*shader);
                }
                else if (pc.ShapeType == PhysicsShapeType::Capsule) {
                    cylinderModel->Draw(*shader);
                }
            }
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//Renderer2D::UseShader(ShaderManager::Get("2d"));


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);

		// render final image 
		{
			auto shader = ShaderManager::Get("2dimage");

			shader->Bind();

			gvao.Bind();
			gebo.Bind();

			app.finalcolorTexture->Bind(0);
			app.finalbloomblurred->Bind(1);
			app.blurredssaobuffer2->Bind(2);
			app.DepthBuffer->Bind(3);

			shader->SetInt("teximage", 0);
			shader->SetInt("bloomimage", 1);
			shader->SetInt("ssaoimage", 2);
			shader->SetInt("depthimage", 3);

			unsigned int ind = Application::s_renderoutput;

			shader->SetInt("renderoutput", ind);
			

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		//s_MainSetting.Render();

		glEnable(GL_DEPTH_TEST);
	}

}
// ...existing code...

void Scene::Render3DModels(std::shared_ptr<Shader> &shader, glm::mat4 viewproj, float ts, bool drawline) {
    // Cache model pointers (avoid repeated ModelManager::GetModel calls)
    static std::shared_ptr<Model> cubeModel = nullptr;
    static std::shared_ptr<Model> sphereModel = nullptr;
    static std::shared_ptr<Model> cylinderModel = nullptr;
    
    if (!cubeModel) cubeModel = ModelManager::GetModel("res/cube.obj");
    if (!sphereModel) sphereModel = ModelManager::GetModel("res/sphere.obj");
    if (!cylinderModel) cylinderModel = ModelManager::GetModel("res/cylinder.obj");

    for (auto e : entities) {

        if (e->HasComponent<Model3DComponent>()) {
            auto& mc = e->GetComponent<Model3DComponent>();
            auto& tc = e->GetComponent<TransformComponent>();

			if (drawline && mc.outline == false)
				continue;

            shader->SetInt("bloom", mc.bloom);
            bool rendered = false;

            if (e->HasComponent<Animation3DComponent>()) {
                auto& ac = e->GetComponent<Animation3DComponent>();

                if (ac.activeanimationindex != 0 && mc.mModel != nullptr) {

					if (mc.outline && !drawline) {
						glEnable(GL_DEPTH_TEST);
						glStencilFunc(GL_ALWAYS, 1, 0xFF);
						glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
						glStencilMask(0xFF);
					}
					else if (mc.outline == false) {
						glStencilMask(0x00);
					}
					else if(mc.outline) {
						glStencilMask(0x00);
						glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
						glDisable(GL_DEPTH_TEST);
					}
					

                    auto ce = GetPrimaryCameraEntity();
                    auto& cc = ce->GetComponent<CameraComponent>();
					Application::captureshadowmap = true;
                    shader->SetInt("isAnimated", 1);

                    ac.UpdateAnimation(ts);

					UpdateLightStatus(tc.position);
                    
					
                    auto bones = ac.GetFinalBoneMetrices();

					
                    int bonecount = bones.size();
                    for (unsigned int i = 0; i < bonecount; i++) {
                        // Use sprintf instead of std::string concatenation (faster)
                        char varname[64];
                        snprintf(varname, sizeof(varname), "finalbonesmetrices[%u]", i);
                        shader->SetMat4(varname, bones[i]);
                    }

					glm::mat4 model = tc.worldtransform;

					if (drawline) {
						TransformComponent newtc = tc;
						newtc.scale *= 1.02f;

						model = tc.worldtransform * glm::inverse(tc.getmatrix()) * newtc.getmatrix();

					}

					//ac.SetBoneMetrices(shader);

                    shader->Bind();
                    shader->SetMat4("viewproj", viewproj);
                    shader->SetMat4("model", model);
					shader->SetInt("bloom", mc.bloom);
                    mc.mModel->Draw(*shader);

                    rendered = true;
                }
            }

            if (mc.mModel != nullptr && !rendered) {

				if (mc.outline && !drawline) {
					glEnable(GL_DEPTH_TEST);
					glStencilFunc(GL_ALWAYS, 1, 0xFF);
					glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
					glStencilMask(0xFF);
				}
				else if (mc.outline == false) {
					glStencilMask(0x00);
				}
				else if (mc.outline) {
					glStencilMask(0x00);
					glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
					glDisable(GL_DEPTH_TEST);
				}
				

				glm::mat4 model = tc.worldtransform;

				if (drawline) {

					TransformComponent newtc = tc;
					newtc.scale *= 1.02f;

					model = tc.worldtransform * glm::inverse(tc.getmatrix()) * newtc.getmatrix();

				}

                shader->SetInt("isAnimated", 0);
                shader->Bind();
                shader->SetMat4("viewproj", viewproj);
                shader->SetMat4("model", model);
				shader->SetInt("bloom", mc.bloom);
                mc.mModel->Draw(*shader);
            }

			glEnable(GL_DEPTH_TEST);
			glStencilMask(0xFF);
			glStencilFunc(GL_ALWAYS, 0, 0xFF);
        }
    }
}