#pragma once

#include "EntityScript.h"
#include "RollButton.h"

struct Dice : EntityScript {

	float changetime = 0.01f;

	float time = 0.0f;

	unsigned int index = 0;

	SubTexture number_textures[6];

	RollButton* m_rollbutton;

	std::shared_ptr<Entity> rollbuttonentity;

	void Start() {

		for (unsigned int i = 0; i < 6; i++) {
			SubTexture a;
			a.index = 5;
			a.coords1 = { i * (1.0f / 6), 1.0f };
			a.coords2 = { (i + 1) * (1.0f / 6), 0.0f };
			number_textures[i] = a;
		}

		rollbuttonentity = SceneManager::GetEntityByName("RollButton");
	}

	void Update(float ts) {

		auto& spc = GetComponent<SpriteRendererComponent>();
		spc.m_subtexture = number_textures[index];

		auto& sc = rollbuttonentity->GetComponent<ScriptComponent>();
		m_rollbutton = (RollButton*)sc.script;

		if (m_rollbutton->rolling) {
			time += ts;

			if (time > changetime) {
				time = time - changetime;

				// increase dice number
				index++;

				if (index > 6) {
					index = 0;
				}
			}
		}
		else {
			auto& nc = GetComponent<NameComponent>();
			if (nc.name == "dice1")
				index = m_rollbutton->dice1 - 1;
			else if (nc.name == "dice2")
				index = m_rollbutton->dice2 - 1;
		}
	}
};