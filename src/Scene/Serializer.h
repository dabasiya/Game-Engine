#pragma once

#include "Scene.h"

#include <string>

namespace YAML {
	class Node;
	class Emitter;
};


struct Serializer {

	std::shared_ptr<Scene> m_scene;

	std::string filepath;

	Serializer() {}
	Serializer(std::shared_ptr<Scene>& a_scene);

	// save scene data in file as YAML
	void Serialize(const std::string& path);

	// load scene data from YAML file
	void Deserialize(const std::string& path);

	static void SerializeEntity(YAML::Emitter& out, std::shared_ptr<Entity> e);
	static void SerializeEntityHierarchy(YAML::Emitter& out, std::shared_ptr<Entity> e);

	static void DeserializeEntity(std::shared_ptr<Entity> deserializedentity, YAML::Node& e);
	static void DeserializeEntityHierarchy(std::shared_ptr<Entity> deserializedentity, YAML::Node& e);
};
