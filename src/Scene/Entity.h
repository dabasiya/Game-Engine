#pragma once

#include "Scene.h"

// later removed
#include <iostream>


struct Entity : public std::enable_shared_from_this<Entity> {

    entt::entity id;

    std::shared_ptr<Scene> m_scene;

    // entity functions

    template<typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        if (HasComponent<T>())
            std::cout << "component already exist" << std::endl;
        return m_scene->m_registry.emplace<T>(id, std::forward<Args>(args)...);
    }

    template<typename T>
    bool HasComponent() {
        if (!m_scene)
            return false;
        return m_scene->m_registry.any_of<T>(id);
    }

    template<typename T>
    T& GetComponent() {
        if (!HasComponent<T>())
            std::cout << "component doesn't exist" << std::endl;
        return m_scene->m_registry.get<T>(id);
    }

    template<typename T>
    void RemoveComponent() {
        std::cout << "component removed" << std::endl;
        m_scene->m_registry.remove<T>(id);
    }

    std::shared_ptr<Entity> CreateChildEntity(const std::string& name);

};
