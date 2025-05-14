#include "entityscript.h"

#include <box2d/box2d.h>
#include "objectid.h"
#include <windowsinput.h>
#include "ContactListener.h"
#include <Scene/SceneManager.h>

#include <glm/gtx/compatibility.hpp>

struct player : EntityScript {

	float player_minimum_size = 0.25f;
	float player_maximum_size = 1.75f;

	float player_feet_size = 0.1f;

	float playerspeed = 1.4f * 1.5f;

	// this is for storing player facing in left or right
	// so animation can be done properly
	bool face_forward = true;

	// this is for check its grounded or not for jump
	bool is_grounded = true;


	float player_size = 1.0f;

	float player_final_size = 1.0f;

	float player_size_change = 1.2f;

	bool player_died = false;

	float jump_mul = 0.5f;

	float player_size_change_initial = 0.0f;

	// this is for player respawning when it died
	float time_after_died = 0.0f; // this is for how much time passed after player died
	float respawn_time_after_died = 3.0f; // this is time for player respawn after this time when it died

	b2Fixture* feetfixture;
	b2Fixture* bodyfixture;

	// for jump player after kill enemy
	bool killed_enemy = false;
	bool is_player_on_air_after_kill_enemy = false;

	float player_initial_scale_x = 0.3333f;
	float player_initial_scale_y = 0.7833f;

	float time_after_player_size_change = 0.0f, player_size_change_time = 0.2f;


	// for showing contact count

	Entity contactcountentity;
	bool contactcountentityfound = false;

	Entity enteroptionentity;


	float enteroptionsize = 0.3f;

	bool is_entered_to_enteroption = false;
	float entered_to_enteroption_timer = 0.0f;
	float enter_option_opacity_change_time = 1.0f;
	float initial_opacity = 0.0f;


	Entity Player_Fading_Frames[30];
	unsigned int playerfadingframeindex = 0;

	void Start() override {

		m_entity.m_scene->physicsworld->SetContactListener(&cl);

		setplayerfeetsensor();

		cl.count = 0;
		cl.onjumper = false;

		cl.player_die = false;

		contactcountentity = SceneManager::GetEntityByName("contactcount");
		enteroptionentity = SceneManager::GetEntityByName("enteroption");

		for (unsigned int i = 0; i < 30; i++) {
			std::string name = "PlayerFadingFrame" + std::to_string(i);
			Player_Fading_Frames[i] = SceneManager::GetEntityByName(name);
		}
	}

	void setplayerfeetsensor() {
		auto& transform = GetComponent<TransformComponent>();

		b2PolygonShape pshape;
		pshape.SetAsBox(player_initial_scale_x * 0.2f * player_final_size / 2.0f, player_feet_size * player_final_size, b2Vec2(0.0f, -(player_initial_scale_y * player_final_size / 2)), 0.0f);

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
		cl.player_height = tcp.scale.y;

		// player input
		RigidBody2DComponent& rbcp = m_entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rbcp.body;

		time_after_player_size_change += time;

		is_grounded = cl.isGrounded();

		b2Vec2 currentvelocity = body->GetLinearVelocity();

		bool isanykeypressed = false;


		updateplayersize();

		// add fading frame
		if (playerfadingframeindex < 30) {
			if (Player_Fading_Frames[playerfadingframeindex].m_scene != nullptr) {
				auto& psp = Player_Fading_Frames[playerfadingframeindex].GetComponent<SpriteRendererComponent>();
				auto& ptc = Player_Fading_Frames[playerfadingframeindex].GetComponent<TransformComponent>();
				psp.m_subtexture = spcp.m_subtexture;
				psp.opacity = 0.5f;
				ptc.position = tcp.position;
				ptc.scale = tcp.scale;
				playerfadingframeindex++;
			}
		}

		// for player movement left or right

		if (Input::KeyPressed(KEY_D) && is_grounded && !player_died) {
			body->SetLinearVelocity(b2Vec2(playerspeed * player_size, currentvelocity.y));
			face_forward = true;
			isanykeypressed = true;
			agcp.animations["run"].Update(time);
			spcp.m_subtexture = agcp.animations["run"].getframe();
			Application::s_SoundManager->PlayAudio("res/walk.mp3", LOOP);
		}

		if (Input::KeyPressed(KEY_A) && is_grounded && !player_died) {
			body->SetLinearVelocity(b2Vec2(-playerspeed * player_size, currentvelocity.y));
			face_forward = false;
			isanykeypressed = true;
			agcp.animations["run"].Update(time);
			spcp.m_subtexture = agcp.animations["run"].getframe();
			Application::s_SoundManager->PlayAudio("res/walk.mp3", LOOP);
		}

		// for player jump
		if (Input::KeyPressed(KEY_SPACE) && is_grounded && !player_died) {
			isanykeypressed = true;
			float mass = body->GetMass();
			body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, mass * player_size), true);
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
			std::cout << "player died" << std::endl;
			auto& bc2d = GetComponent<BoxCollider2DComponent>();
			bc2d.fixture->SetSensor(true);
			float jumpstrength = 0.05f;
			float extrasize = player_size - 1.0f;
			if (extrasize > 0.0f) {
				jumpstrength += (extrasize * 0.5f) / 4.0f;
			}

			b2Vec2 velocity = body->GetLinearVelocity();
			velocity.x = 0.0f;
			body->SetLinearVelocity(velocity);
			body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, jumpstrength * 4.0f * player_size * player_size), true);
		}
		player_died = cl.player_die;


		// for update time of time_after_died variable
		// so after fixed time player can be respawned
		if (player_died) {
			time_after_died += time;
		}


		// this is for player respawning
		// if time after died is become greater than respawn_time_after_died then player should be
		// respawn
		// and set box fixture sensor false so it can be collided
		if (time_after_died > respawn_time_after_died) {
			player_died = false;
			player_size = 1.0f;
			player_final_size = 1.0f;
			auto& tc = GetComponent<TransformComponent>();
			tc.position = glm::vec3(cl.player_respawn_position.x, cl.player_respawn_position.y, tc.position.z);
			body->SetTransform({ tc.position.x, tc.position.y }, 0.0f);
			auto& bc2d = GetComponent<BoxCollider2DComponent>();
			bc2d.fixture->SetSensor(false);
			cl.player_die = false;
			time_after_died = 0.0f;
			is_grounded = false;

			auto& transform = GetComponent<TransformComponent>();

			transform.scale.x = 0.3333f;
			transform.scale.y = 0.7833f;

			face_forward = true;

			cl.reset_player_collision();

			RigidBody2DComponent& rbcp = m_entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rbcp.body;
			body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
			updateplayerphysicsproperties();
			setplayerfeetsensor();

		}


		// if player showing at backward
		if (!face_forward) {
			float temp = spcp.m_subtexture.coords1.x;
			spcp.m_subtexture.coords1.x = spcp.m_subtexture.coords2.x;
			spcp.m_subtexture.coords2.x = temp;
		}


		// if player killed any enemy
		if (cl.is_enemy_died && !player_died) {
			std::cout << "player jump after enemy died" << std::endl;
			float mass = body->GetMass();
			body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, mass * player_size * 5.0f), true);
			is_grounded = false;
		}

		cl.reset_died_enemies_info();

		if (cl.player_entered_enter_option && !is_entered_to_enteroption) {
			is_entered_to_enteroption = true;
			auto& spc = enteroptionentity.GetComponent<SpriteRendererComponent>();
			entered_to_enteroption_timer = (spc.opacity < 0.0f) ? 0.0f : spc.opacity;
		}
		else if (!cl.player_entered_enter_option && is_entered_to_enteroption) {
			is_entered_to_enteroption = false;
			auto& spc = enteroptionentity.GetComponent<SpriteRendererComponent>();
			entered_to_enteroption_timer = (spc.opacity > 1.0f) ? 1.0f : spc.opacity;
		}

		if (contactcountentity.m_scene != nullptr)
			setcontactcount();

		if (enteroptionentity.m_scene != nullptr)
			updateenteroptionposition(time);
	}

	void OnEvent(Event& e) {

		if (!player_died) {

			// when mouse scrolled then player size is updated

			if (e.is(Event_Type::Mouse_Scrolled) && !is_grounded) {
				MouseScrolledEvent* me = (MouseScrolledEvent*)&e;
				auto& transform = GetComponent<TransformComponent>();

				time_after_player_size_change = 0.0f;

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
					player_final_size *= player_size_change;
					player_size_change_initial = player_size;
					playerfadingframeindex = 0;
				}
				else if (me->yoffset < 0.0f && isvalid) {
					player_final_size /= player_size_change;
					player_size_change_initial = player_size;
					playerfadingframeindex = 0;
				}


				updateplayerphysicsproperties();
			}
		}
	}

	void updateplayersize() {
		auto& transform = GetComponent<TransformComponent>();

		if (player_size != player_final_size) {
			player_size = interpolate(player_size_change_initial, player_final_size, time_after_player_size_change, player_size_change_time);
			transform.scale.x = player_initial_scale_x * player_size;
			transform.scale.y = player_initial_scale_y * player_size;
		}
	}

	float interpolate(float a, float b, float t, float interpolatetime) {
		if (t >= interpolatetime)
			return b;

		float n = t / interpolatetime;
		return ((a * (1 - n)) + (n * b));
	}


	void updateplayerphysicsproperties() {

		auto& transform = GetComponent<TransformComponent>();
		auto& bc2d = GetComponent<BoxCollider2DComponent>();
		auto& rb2d = GetComponent<RigidBody2DComponent>();

		b2Vec2 velocity = rb2d.body->GetLinearVelocity();
		float mass = rb2d.body->GetMass();
		rb2d.body->DestroyFixture(feetfixture);
		rb2d.body->DestroyFixture(bc2d.fixture);

		b2PolygonShape pshape;
		pshape.SetAsBox(player_initial_scale_x * player_final_size * bc2d.size.x, player_initial_scale_y * player_final_size * bc2d.size.y);

		b2FixtureDef def;
		def.density = bc2d.density;
		def.shape = &pshape;
		def.friction = bc2d.friction;
		bc2d.fixture = rb2d.body->CreateFixture(&def);
		bc2d.fixture->SetUintData(bc2d.objectid);

		setplayerfeetsensor();

		float newmass = rb2d.body->GetMass();

		float gain = newmass / mass;

		if (player_size < player_final_size) {
			float diff = -(player_final_size) / 2.0f;
			rb2d.body->ApplyLinearImpulseToCenter({ 0.0f, diff }, true);
		}
	}


	void setcontactcount() {
		auto& fc = contactcountentity.GetComponent<FontRendererComponent>();
		fc.text = "";
		fc.text += std::to_string(cl.count);
	}

	void updateenteroptionposition(float ts) {
		auto& pt = GetComponent<TransformComponent>();
		auto& ot = enteroptionentity.GetComponent<TransformComponent>();

		ot.scale.x = enteroptionsize * player_size;
		ot.scale.y = enteroptionsize * player_size;

		ot.position.x = pt.position.x + 0.4f * player_size;
		ot.position.y = pt.position.y + 0.4f * player_size;

		if (is_entered_to_enteroption && entered_to_enteroption_timer < 1.0f) {
			auto& spc = enteroptionentity.GetComponent<SpriteRendererComponent>();
			float opacity = glm::lerp(0.0f, 1.0f, entered_to_enteroption_timer);
			spc.opacity = opacity;
			entered_to_enteroption_timer += ts;
		}

		else if (!is_entered_to_enteroption && entered_to_enteroption_timer < 1.0f) {
			auto& spc = enteroptionentity.GetComponent<SpriteRendererComponent>();
			float opacity = glm::lerp(0.0f, 1.0f, entered_to_enteroption_timer);
			spc.opacity = opacity;
			entered_to_enteroption_timer -= ts;
		}
	}
};
