#include "entityscript.h"

#include <box2d/box2d.h>
#include "objectid.h"
#include <windowsinput.h>
#include "ContactListener.h"


struct player : EntityScript {

	float player_minimum_size = 0.25f;
	float player_maximum_size = 2.0f;

	float player_feet_size = 0.1f;

	float playerspeed = 1.4f * 1.5f;

        // this is for storing player facing in left or right
        // so animation can be done properly
	bool face_forward = true;

        // this is for check its grounded or not for jump
	bool is_grounded = true;


	float player_size = 1.0f;

	float player_size_change = 1.2f;

	bool player_died = false;

	float jump_mul = 0.1f;

        // this is for player respawning when it died
        float time_after_died = 0.0f; // this is for how much time passed after player died
        float respawn_time_after_died = 3.0f; // this is time for player respawn after this time when it died

	b2Fixture* feetfixture;
	b2Fixture* bodyfixture;

	// for jump player after kill enemy
	bool killed_enemy = false;
	bool is_player_on_air_after_kill_enemy = false;

	void Start() override {

		m_entity.m_scene->physicsworld->SetContactListener(&cl);

		setplayerfeetsensor();

		cl.count = 0;
		cl.onjumper = false;

		cl.player_die = false;
	}

	void setplayerfeetsensor() {
		auto& transform = GetComponent<TransformComponent>();

		b2PolygonShape pshape;
		pshape.SetAsBox(player_feet_size, player_feet_size, b2Vec2(0.0f, -(transform.scale.y / 2)), 0.0f);

		b2FixtureDef fdef;
		fdef.shape = &pshape;
		fdef.density = 1.0f;
		fdef.isSensor = true;

		auto& body = GetComponent<RigidBody2DComponent>();
		b2Body* bbody = body.body;
		feetfixture = bbody->CreateFixture(&fdef);
		feetfixture->SetUintData(objectid::Player_Feet);

	}

	void Update(float time) override {

		// for player animation
		AnimationGroupComponent& agcp = m_entity.GetComponent<AnimationGroupComponent>();
		SpriteRendererComponent& spcp = m_entity.GetComponent<SpriteRendererComponent>();

		TransformComponent& tcp = m_entity.GetComponent<TransformComponent>();

		// player input
		RigidBody2DComponent& rbcp = m_entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rbcp.body;

		body->SetBullet(true);

		is_grounded = cl.isGrounded();

		b2Vec2 currentvelocity = body->GetLinearVelocity();

		bool isanykeypressed = false;

		// for player movement left or right

		if (Input::KeyPressed(KEY_D) && is_grounded) {
			body->SetLinearVelocity(b2Vec2(playerspeed * player_size, currentvelocity.y));
			face_forward = true;
			isanykeypressed = true;
			agcp.animations["run"].Update(time);
			spcp.m_subtexture = agcp.animations["run"].getframe();
			Application::s_SoundManager->PlayAudio("res/walk.mp3", LOOP);
		} 

		if (Input::KeyPressed(KEY_A) && is_grounded) {
			body->SetLinearVelocity(b2Vec2(-playerspeed * player_size, currentvelocity.y));
			face_forward = false;
			isanykeypressed = true;
			agcp.animations["run"].Update(time);
			spcp.m_subtexture = agcp.animations["run"].getframe();
			Application::s_SoundManager->PlayAudio("res/walk.mp3", LOOP);
		}

		// for player jump
		if (Input::KeyPressed(KEY_SPACE) && is_grounded) {
			isanykeypressed = true;
			float jumpstrength = player_size * player_size * jump_mul;
			body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, jumpstrength * player_size), true);
			is_grounded = false;
		}

		// when key A or D not pressed reset run animation frame pointer to 0
		if (!Input::KeyPressed(KEY_D) && !Input::KeyPressed(KEY_A) && is_grounded) {
			agcp.animations["run"].time = 0.0f;
			body->SetLinearVelocity(b2Vec2(0.0f, currentvelocity.y));
		}

		// if any key is not pressed the update animation of idle animation
		// and stop walk audio
		if (!isanykeypressed) {
			agcp.animations["idle"].Update(time);
			spcp.m_subtexture = agcp.animations["idle"].getframe();
			Application::s_SoundManager->StopAudio("res/walk.mp3");
		}

		// if player is not grounded then jump frame set
		if (!is_grounded) {
			spcp.m_subtexture = agcp.animations["jump"].getframe();
		}

		// when player is died this block executed
		if (cl.player_die && !player_died) {
			auto& bc2d = GetComponent<BoxCollider2DComponent>();
			bc2d.fixture->SetSensor(true);
			float jumpstrength = 0.05f;
			float extrasize = player_size - 1.0f;
			if (extrasize > 0.0f) {
				jumpstrength += extrasize * 0.5f;
			}
			body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, jumpstrength * 5.0f * player_size * player_size), true);
		}
		player_died = cl.player_die;


                // for update time of time_after_died variable
                // so after fixed time player can be respawned
                if(player_died) {
                        time_after_died += time;
                }


                // this is for player respawning
                // if time after died is become greater than respawn_time_after_died then player should be
                // respawn
                // and set box fixture sensor false so it can be collided
                if(time_after_died > respawn_time_after_died) {
                        player_died = false;
                        auto& tc = GetComponent<TransformComponent>();
                        tc.position = glm::vec3(cl.player_respawn_position, tc.position.z);
                        body->SetTransform({tc.position.x, tc.position.y}, 0.0f);
                        auto& bc2d = GetComponent<BoxCollider2DComponent>();
			bc2d.fixture->SetSensor(false);
                        cl.player_die = false;
                        time_after_died = 0.0f;
                        cl.count = 0;                   
                }


		// if player showing at backward
		if (!face_forward) {
			float temp = spcp.m_subtexture.coords1.x;
			spcp.m_subtexture.coords1.x = spcp.m_subtexture.coords2.x;
			spcp.m_subtexture.coords2.x = temp;
		}


		// if player killed any enemy
		if (cl.is_enemy_died) {
			float jumpstrength = 0.05f;
			float extrasize = player_size - 1.0f;
			if (extrasize > 0.0f) {
				jumpstrength += extrasize * 0.5f;
			}
			body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, jumpstrength * 5 * player_size * player_size), true);
			is_grounded = false;
		}

		cl.reset_died_enemies_info();
	}

	void OnEvent(Event& e) {

		// when mouse scrolled then player size is updated

		if (e.is(Event_Type::Mouse_Scrolled) && !is_grounded) {
			MouseScrolledEvent* me = (MouseScrolledEvent*)&e;
			auto& transform = GetComponent<TransformComponent>();

			bool isvalid = true;
			if (me->yoffset > 0.0f) {
				if (player_size * player_size_change > player_maximum_size)
					isvalid = false;
			}
			else if (me->yoffset < 0.0f) {
				if (player_size / player_size_change < player_minimum_size)
					isvalid = false;
			}


			if (me->yoffset > 0.0f && isvalid) {
				transform.scale.x *= player_size_change;
				transform.scale.y *= player_size_change;
				player_size *= player_size_change;
			}
			else if (me->yoffset < 0.0f && isvalid) {
				transform.scale.x /= player_size_change;
				transform.scale.y /= player_size_change;
				player_size /= player_size_change;
			}


			auto& bc2d = GetComponent<BoxCollider2DComponent>();
			auto& rb2d = GetComponent<RigidBody2DComponent>();

			rb2d.body->DestroyFixture(feetfixture);
			rb2d.body->DestroyFixture(bc2d.fixture);

			b2PolygonShape pshape;
			pshape.SetAsBox(transform.scale.x * bc2d.size.x, transform.scale.y * bc2d.size.y);

			b2FixtureDef def;
			def.density = bc2d.density;
			def.shape = &pshape;
			def.friction = bc2d.friction;
			bc2d.fixture = rb2d.body->CreateFixture(&def);
			bc2d.fixture->SetUintData(bc2d.objectid);

			setplayerfeetsensor();

			rb2d.body->SetGravityScale(player_size * player_size);
		}
	}
};
