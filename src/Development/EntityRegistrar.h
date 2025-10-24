#pragma once


#include <Scene/Scene.h>

struct EntityRegistrar {

	// it store entities at run time
	std::shared_ptr<Scene> m_scene;

	EntityRegistrar();

	void Load();

	void Save();

	// Add Entity in EntityRegistrar 
	// this adds new entity in EntityRegistrars Scene pointer
	void AddEntity(std::shared_ptr<Entity> e);

	// copy entities component into other entity
	bool CopyEntity(std::shared_ptr<Entity> dstEntity, std::shared_ptr<Entity> srcEntity);

	// Add Entity into main scene
	void AddEntityIntoScene(std::shared_ptr<Scene>& scene, std::shared_ptr<Entity> e);
};