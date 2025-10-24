#include "Entity.h"
#include "Components.h"

std::shared_ptr<Entity> Entity::CreateChildEntity(const std::string& name) {
    std::shared_ptr<Entity> e = std::make_shared<Entity>();
    e->id = m_scene->m_registry.create();
    e->m_scene = m_scene;
    e->AddComponent<NameComponent>(name);
    auto& rc = e->AddComponent<RelationshipComponent>();
    e->AddComponent<TransformComponent>(glm::vec3(0.0f));
    rc.parentEntity = shared_from_this();

    auto& rrc = GetComponent<RelationshipComponent>();
    rrc.childEntities.push_back(e);
    return e;
}