#pragma once

#include <optional>
#include "Components.h"


struct CopiedEntity {

	std::optional<NameComponent> nc;
	std::optional<TransformComponent> tc;
	std::optional<CameraComponent> cc;
	std::optional<SpriteRendererComponent> spc;
	std::optional<FontRendererComponent> fc;
	std::optional<ScriptComponent> sc;
	std::optional<AnimationGroupComponent> agc;
	std::optional<RigidBody2DComponent> rbc;
	std::optional<BoxCollider2DComponent> bcc;
	std::optional<ChainShapeColliderComponent> csc;
	std::optional<ParticleGeneratorComponent> pgc;
	std::optional<Model3DComponent> m3c;
	std::optional<LightComponent> lc;
	std::optional<PhysicsComponent> pc;
	std::optional<Animation3DComponent> a3c;
};
