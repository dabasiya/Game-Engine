#pragma once

#include <Scene/Entity.h>

struct EntityScript {

	std::shared_ptr<Entity> m_entity;

	template<typename T>
	T& GetComponent() {
		return m_entity->GetComponent<T>();
	}

	virtual void Start() {}
	virtual void Update(float time) {}
	virtual void OnEvent(Event& e) {}
};
