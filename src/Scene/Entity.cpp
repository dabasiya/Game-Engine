#include "Entity.h"
#include "Components.h"

std::shared_ptr<Entity> Entity::CreateChildEntity(const std::string& name) {
    std::shared_ptr<Entity> e = m_scene->CreateEntity(name);
    auto& rc = e->GetComponent<RelationshipComponent>();
    rc.parentEntity = shared_from_this();

    auto& rrc = GetComponent<RelationshipComponent>();
    rrc.childEntities.push_back(e);
    return e;
}

