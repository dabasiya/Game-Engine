#include "entityscript.h"

#include <box2d/box2d.h>
#include <windowsinput.h>

class ContactListener : public b2ContactListener {
public:
	bool isGrounded = true;

	void BeginContact(b2Contact* contact) override {
		unsigned int bodyuserdataa = contact->GetFixtureA()->GetBody()->GetuintData();
		unsigned int bodyuserdatab = contact->GetFixtureB()->GetBody()->GetuintData();

		if (bodyuserdataa == 1 || bodyuserdatab == 1) {
			isGrounded = true;
		}
	}

	void EndContact(b2Contact* contact) override {
		unsigned int bodyuserdataa = contact->GetFixtureA()->GetBody()->GetuintData();
		unsigned int bodyuserdatab = contact->GetFixtureB()->GetBody()->GetuintData();

		if (bodyuserdataa == 1 || bodyuserdatab == 1) {
			isGrounded = false;
		}
	}
};

struct player : entityscript {

	float playerspeed = 0.7f;

	bool face_forward = true;

	bool is_grounded = true;

	float aftertime_change_jump_velocity = 0.35f;

	float jumptime = 0;

	ContactListener cl;

	entity tree_bg;
	entity hill_bg;

	float tree_speed = 1.0f;
	float hill_speed = 0.5f;

	void start() override {

		m_entity.m_scene->physicsworld->SetContactListener(&cl);

		auto view = m_entity.m_scene->m_registry.view<namecomponent>();

		for (auto e : view) {
			auto nc = view.get<namecomponent>(e);
			if (nc.name == "hill_bg") {
				hill_bg = { e, m_entity.m_scene };
			}
			else if (nc.name == "tree_bg") {
				tree_bg = { e, m_entity.m_scene };
			}
		}

	}

	void update(float time) override {

		// for player animation
		animationgroupcomponent& agcp = m_entity.getcomponent<animationgroupcomponent>();
		spriterenderercomponent& spcp = m_entity.getcomponent<spriterenderercomponent>();

		transformcomponent& tcp = m_entity.getcomponent<transformcomponent>();

		glm::vec3& treepos = tree_bg.getcomponent<transformcomponent>().position;
		glm::vec3& hillpos = hill_bg.getcomponent<transformcomponent>().position;

		// player input
		rigidbody2dcomponent& rbcp = m_entity.getcomponent<rigidbody2dcomponent>();
		b2Body* body = (b2Body*)rbcp.body;

		is_grounded = cl.isGrounded;

		b2Vec2 currentvelocity = body->GetLinearVelocity();

		bool isanykeypressed = false;
		if (input::keypressed(KEY_D) && is_grounded) {
			body->SetLinearVelocity(b2Vec2(playerspeed, currentvelocity.y));
			face_forward = true;
			isanykeypressed = true;
			agcp.animations["run"].update(time);
			spcp.sub_texture = agcp.animations["run"].getframe();
		}
		if (input::keypressed(KEY_A) && is_grounded) {
			body->SetLinearVelocity(b2Vec2(-playerspeed, currentvelocity.y));
			face_forward = false;
			isanykeypressed = true;
			agcp.animations["run"].update(time);
			spcp.sub_texture = agcp.animations["run"].getframe();
		}
		if (input::keypressed(KEY_SPACE) && is_grounded) {
			jumptime = 0;
			isanykeypressed = true;
			//body->SetLinearVelocity(b2Vec2(currentvelocity.x, 1.2f));
			body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, 0.05f), true);
			is_grounded = false;
		}
		if (!input::keypressed(KEY_D) && !input::keypressed(KEY_A) && is_grounded) {
			agcp.animations["run"].time = 0.0f;
			body->SetLinearVelocity(b2Vec2(0.0f, currentvelocity.y));
		}

		if (!isanykeypressed) {
			agcp.animations["idle"].update(time);
			spcp.sub_texture = agcp.animations["idle"].getframe();
		}

		if (!is_grounded) {
			jumptime += time;
			if (jumptime > aftertime_change_jump_velocity) {
				b2Vec2 v = body->GetLinearVelocity();
				v.y = v.y - (2.5f * time);
				body->SetLinearVelocity(v);
			}
			spcp.sub_texture = agcp.animations["jump"].getframe();
		}



		// if player showing at backward
		if (!face_forward) {
			float temp = spcp.sub_texture.coords1.x;
			spcp.sub_texture.coords1.x = spcp.sub_texture.coords2.x;
			spcp.sub_texture.coords2.x = temp;
		}

		if (tcp.position.x > -6.30f)
		{
			b2Vec2 velocity = body->GetLinearVelocity();
			treepos.x -= velocity.x * time;
			hillpos.x -= velocity.x * 0.5f * time;
		}

	}
};
