#include "EntityRegistrar.h"

#include <string>
#include <sstream>

#include <Scene/Serializer.h>

#include <syaml.h>

#include <Scene/Entity.h>
#include <Scene/Components.h>

EntityRegistrar::EntityRegistrar() {
	m_scene = std::make_shared<Scene>();
	Load();
}

void EntityRegistrar::Load() {
	std::ifstream ifl("res/EntityRegistrar.scene");
	std::stringstream strstream;
	strstream << ifl.rdbuf();

	YAML::Node data = YAML::Load(strstream.str());

	if (!data["Scene"])
		return;

	auto entities = data["Entities"];
	if (entities) {
		for (auto e : entities) {

			uint32_t id = e["Entity"].as<uint32_t>();

			// name component
			auto nc = e["NameComponent"];
			std::string name = nc["name"].as<std::string>();


			std::shared_ptr<Entity> deserializedentity = m_scene->CreateEntity(name);

			Serializer::DeserializeEntity(deserializedentity, e);

			auto& tc = deserializedentity->GetComponent<TransformComponent>();
			tc.position = glm::vec3(0.0f);
			tc.rotation = glm::vec3(0.0f);
		}
	}
}

void EntityRegistrar::Save() {
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Scene" << YAML::Value << "Undefined";

	out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;


	auto view = m_scene->m_registry.view<TransformComponent>();
	for (auto entityid : view) {
		std::shared_ptr<Entity> e = std::make_shared<Entity>();
		e->id = entityid;
		e->m_scene = m_scene;

		Serializer::SerializeEntity(out, e);
	}

	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::ofstream os("res/EntityRegistrar.scene");
	os << out.c_str();
	os.close();
}

bool EntityRegistrar::CopyEntity(std::shared_ptr<Entity> dstEntity, std::shared_ptr<Entity> srcEntity) {
	
	// if scene not exist in entity object then return false
	if (dstEntity->m_scene == nullptr || srcEntity->m_scene == nullptr)
		return false;

	// add all components
	
	// NameComponent and TransformComponent already exist so we have to fetch from dst entity and set its value

	//										NameComponent
	auto& d_nc = dstEntity->GetComponent<NameComponent>();
	auto& s_nc = srcEntity->GetComponent<NameComponent>();

	d_nc = s_nc;

	//										TransformComponent
	auto& d_tc = dstEntity->GetComponent<TransformComponent>();
	auto& s_tc = srcEntity->GetComponent<TransformComponent>();

	d_tc = s_tc;

	//										CameraComponent
	if (srcEntity->HasComponent<CameraComponent>()) {
		auto& d_cc = dstEntity->AddComponent<CameraComponent>();
		auto& s_cc = srcEntity->GetComponent<CameraComponent>();

		d_cc = s_cc;
	}

	//										SpriteRendererComponent
	if (srcEntity->HasComponent<SpriteRendererComponent>()) {
		auto& d_sc = dstEntity->AddComponent<SpriteRendererComponent>();
		auto& s_sc = srcEntity->GetComponent<SpriteRendererComponent>();

		d_sc = s_sc;
	}

	//										FontRendererComponent
	if (srcEntity->HasComponent<FontRendererComponent>()) {
		auto& d_fc = dstEntity->AddComponent<FontRendererComponent>();
		auto& s_fc = srcEntity->GetComponent<FontRendererComponent>();

		d_fc = s_fc;
	}

	//										ScriptComponent
	if (srcEntity->HasComponent<ScriptComponent>()) {
		auto& d_sc = dstEntity->AddComponent<ScriptComponent>("");
		auto& s_sc = srcEntity->GetComponent<ScriptComponent>();

		d_sc = s_sc;
	}

	//										AnimationGroupComponent
	if (srcEntity->HasComponent<AnimationGroupComponent>()) {
		auto& d_agc = dstEntity->AddComponent<AnimationGroupComponent>();
		auto& s_agc = srcEntity->GetComponent<AnimationGroupComponent>();

		d_agc = s_agc;
	}

	//										RigidBody2DComponent
	if (srcEntity->HasComponent<RigidBody2DComponent>()) {
		auto& d_rc = dstEntity->AddComponent<RigidBody2DComponent>();
		auto& s_rc = dstEntity->GetComponent<RigidBody2DComponent>();

		d_rc = s_rc;
	}

	//										BoxCollider2DComponent
	if (srcEntity->HasComponent<BoxCollider2DComponent>()) {
		auto& d_bc = dstEntity->AddComponent<BoxCollider2DComponent>();
		auto& s_bc = dstEntity->GetComponent<BoxCollider2DComponent>();

		d_bc = s_bc;
	}

	//										ChainShapeColliderComponent
	if (srcEntity->HasComponent<ChainShapeColliderComponent>()) {
		auto& d_cc = dstEntity->AddComponent<ChainShapeColliderComponent>();
		auto& s_cc = dstEntity->GetComponent<ChainShapeColliderComponent>();

		d_cc = s_cc;
	}

	//										ParticleGeneratorComponent
	if (srcEntity->HasComponent<ParticleGeneratorComponent>()) {
		auto& d_pc = dstEntity->AddComponent<ParticleGeneratorComponent>();
		auto& s_pc = dstEntity->GetComponent<ParticleGeneratorComponent>();

		d_pc = s_pc;
	}
}

void EntityRegistrar::AddEntity(std::shared_ptr<Entity> e) {
	std::shared_ptr<Entity> newEntity = m_scene->CreateEntity();

	CopyEntity(newEntity, e);
}

void EntityRegistrar::AddEntityIntoScene(std::shared_ptr<Scene>& scene, std::shared_ptr<Entity> e) {
	std::shared_ptr<Entity> sceneentity = scene->CreateEntity();

	CopyEntity(sceneentity, e);
}