#pragma once

#include "EntityScript.h"
#include <scene/Components.h>
#include "ObjectID.h"

struct EnemyGenerator : EntityScript {

	float timer = 0.0f;

	float enemy_spawn_time = 3.0f;

	void Start() override {

	}

	void Update(float time) override {

		timer += time;

		if (timer > enemy_spawn_time) {
			timer = timer - enemy_spawn_time;
			EmitEnemy();
		}
	}

	void EmitEnemy() {
		Entity enemy = m_entity.m_scene->CreateEntity("Enemy");

		auto& sc = enemy.AddComponent<SpriteRendererComponent>();
		auto& rb2d = enemy.AddComponent<RigidBody2DComponent>();
		auto& bc2d = enemy.AddComponent<BoxCollider2DComponent>();
		auto& tc = enemy.GetComponent<TransformComponent>();
		auto& scc = enemy.AddComponent<ScriptComponent>("enemytest");

		auto& ttc = GetComponent<TransformComponent>();

		tc.position = ttc.position;
		tc.scale.x = 0.4f;
		tc.scale.y = 0.4f;

		sc.type = 1;

		sc.m_subtexture.coords1 = { 0.0f, 1.0f };
		sc.m_subtexture.coords2 = { 1.0f, 0.0f };
		sc.m_subtexture.index = 8;

		rb2d.type = b2BodyType::b2_dynamicBody;

		bc2d.density = 1.0f;
		bc2d.friction = 0.4f;
		bc2d.width = 0.4f;
		bc2d.height = 0.4f;
		bc2d.objectid = objectid::Enemy;

		b2BodyDef bodydef;
		bodydef.type = rb2d.type;
		bodydef.position.Set(tc.position.x, tc.position.y);
		bodydef.fixedRotation = rb2d.fixedrotation;
		b2Body* body = m_entity.m_scene->physicsworld->CreateBody(&bodydef);
		rb2d.body = body;
		body->SetuintData((unsigned int)enemy.id);

		b2Vec2 pos = body->GetPosition();
		body->SetTransform(pos, tc.rotation.z * (3.14f / 180.0f));

		b2PolygonShape polygonshape;
		polygonshape.SetAsBox(bc2d.size.x * tc.scale.x, bc2d.size.y * tc.scale.y);

		b2FixtureDef fixturedef;
		fixturedef.shape = &polygonshape;
		fixturedef.density = bc2d.density;
		fixturedef.friction = bc2d.friction;
		fixturedef.isSensor = bc2d.isSensor;
		bc2d.fixture = body->CreateFixture(&fixturedef);
		bc2d.fixture->SetUintData(bc2d.objectid);

		m_entity.m_scene->reorder_rendering_sequence();

		rb2d.body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, 1.0f), true);
	}
};
