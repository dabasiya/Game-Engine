#pragma once

#include "EntityScript.h"
#include <Scene/Components.h>

#include <WindowsInput.h>

enum Player_Move_State {
	IDLE = 0,
	WALK = 1
};

enum Player_Face_Direction {
	FORWARD = 0, 
	BACK, 
	RIGHT, 
	LEFT
};


struct Player : EntityScript {

	Player_Move_State  m_Playermovestate;
	Player_Face_Direction m_Playerfacedirection;

	float m_Playerspeed = 2.0f;

	void Start() override {

	}

	void Update(float ts) override {

		auto& sc = GetComponent<SpriteRendererComponent>();
		auto& agc = GetComponent<AnimationGroupComponent>();

		auto& ptc = GetComponent<TransformComponent>();

		auto& ctc = m_entity->m_scene->GetPrimaryCameraEntity()->GetComponent<TransformComponent>();

		// player input and move player 
		m_Playermovestate = WALK;

		if (Input::KeyPressed(KEY_W)) {
			m_Playerfacedirection = BACK;
			ptc.position.y += ts * m_Playerspeed;
		}

		else if (Input::KeyPressed(KEY_S)) {
			m_Playerfacedirection = FORWARD;
			ptc.position.y -= ts * m_Playerspeed;
		}

		else if (Input::KeyPressed(KEY_D)) {
			m_Playerfacedirection = RIGHT;
			ptc.position.x += ts * m_Playerspeed;
		}

		else if (Input::KeyPressed(KEY_A)) {
			m_Playerfacedirection = LEFT;
			ptc.position.x -= ts * m_Playerspeed;
		}

		else {
			m_Playermovestate = IDLE;
		}


		// subtexture printing
		{
			if (m_Playermovestate == IDLE) {
				if (m_Playerfacedirection == FORWARD) {
					sc.m_subtexture = agc.animations["forward"].frames[0];
					agc.animations["forward"].time = 0.0f;
				}

				else if (m_Playerfacedirection == BACK) {
					sc.m_subtexture = agc.animations["back"].frames[0];
					agc.animations["back"].time = 0.0f;
				}

				else if (m_Playerfacedirection == RIGHT) {
					sc.m_subtexture = agc.animations["right"].frames[0];
					agc.animations["right"].time = 0.0f;
				}

				else if (m_Playerfacedirection == LEFT) {
					sc.m_subtexture = agc.animations["right"].frames[0];
					SubTexture a = sc.m_subtexture;
					glm::vec2 coord1 = sc.m_subtexture.coords1, coord2 = sc.m_subtexture.coords2;
					sc.m_subtexture = { sc.m_subtexture.index, glm::vec2(coord2.x, coord1.y), glm::vec2(coord1.x, coord2.y) };
					agc.animations["right"].time = 0.0f;
				}
			}

			else if (m_Playermovestate == WALK) {
				if (m_Playerfacedirection == FORWARD) {
					agc.animations["forward"].Update(ts);
					sc.m_subtexture = agc.animations["forward"].getframe();
				}

				else if (m_Playerfacedirection == BACK) {
					agc.animations["back"].Update(ts);
					sc.m_subtexture = agc.animations["back"].getframe();
				}

				else if (m_Playerfacedirection == RIGHT) {
					agc.animations["right"].Update(ts);
					sc.m_subtexture = agc.animations["right"].getframe();
				}

				else if (m_Playerfacedirection == LEFT) {
					agc.animations["right"].Update(ts);
					sc.m_subtexture = agc.animations["right"].getframe();
					SubTexture a = sc.m_subtexture;
					glm::vec2 coord1 = sc.m_subtexture.coords1, coord2 = sc.m_subtexture.coords2;
					sc.m_subtexture = { sc.m_subtexture.index, glm::vec2(coord2.x, coord1.y), glm::vec2(coord1.x, coord2.y) };
				}
			}
		}

		ctc.position.x = ptc.position.x;
		ctc.position.y = ptc.position.y;

	}
};