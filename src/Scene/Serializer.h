#pragma once

#include "Scene.h"

#include <string>

struct Serializer {

	Scene* m_scene;

	std::string filepath;

	Serializer() {}
	Serializer(Scene* a_scene);

	// save scene data in file as YAML
	void Serialize(const std::string& path);

	// load scene data from YAML file
	void Deserialize(const std::string& path);
};
