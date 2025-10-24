#pragma once


#include "EntityScript.h"
#include <Scene/Components.h>

#include <Renderer2D.h>

#include <Scene/SceneManager.h>

#include <ShaderManager.h>

#define MAP_X 32
#define MAP_Y 32

struct Map : EntityScript {

	// subtextures
	SubTexture grass_subtexture = { 3, {0.0f, 0.029f}, {0.02929f, 0.0f} };

	
	unsigned int map[MAP_Y][MAP_Y];

	float tile_x_size = 1.0f;
	float tile_y_size = 1.0f;


	void Start() override {
		
		for (unsigned int i = 0; i < MAP_Y; i++) {
			for (unsigned int j = 0; j < MAP_X; j++) {
				map[i][j] = 1;
			}
		}
	}

	void Update(float ts) override {
		
		auto ce = m_entity.m_scene->GetPrimaryCameraEntity();
		auto& cc = ce.GetComponent<CameraComponent>();


		auto& tc = GetComponent<TransformComponent>();
		auto& ctc = m_entity.m_scene->GetPrimaryCameraEntity().GetComponent<TransformComponent>();

		glm::mat4 viewproj = cc.camerao.viewprojection * glm::inverse(ctc.getmatrix());

		Renderer2D::UseShader(ShaderManager::Get("2d"));
		Renderer2D::BeginScene(viewproj);

		float beginx = tc.position.x - tile_x_size * MAP_X * 0.5f;
		float beginy = tc.position.y + tile_y_size * MAP_Y * 0.5f;

		for (unsigned int i = 0; i < MAP_Y; i++) {
			for (unsigned int j = 0; j < MAP_X; j++) {
				float x = beginx + tile_x_size * j + tile_x_size*0.5f;
				float y = beginy - tile_y_size * i - tile_y_size*0.5f;
				Renderer2D::DrawQuad(glm::vec3(x, y, tc.position.z), { tile_x_size, tile_y_size }, grass_subtexture);
			}
		}
		

		Renderer2D::EndScene();

	}
};