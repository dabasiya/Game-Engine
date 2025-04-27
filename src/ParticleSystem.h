#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <SubTexture.h>


struct ParticleProps {
	glm::vec3 position;
	glm::vec2 velocity = { 0.0f, 0.0f }, velocityvariation = { 0.1f, 0.1f };
	glm::vec4 colorbegin = glm::vec4(1.0f), colorend = glm::vec4(1.0f);
	float sizevariation = 0.0f;
	float sizebegin = 1.0f, sizeend = 0.2f;
	float lifetime = 1.0f;
	SubTexture m_subtexture;
	bool isimage = false;
        bool generate_from_area = false;
        glm::vec2 scale;
        float z_rotation = false;
};

struct Particle {
	glm::vec3 position;
	glm::vec2 velocity;
	glm::vec4 colorbegin, colorend;
	SubTexture m_subtexture;
	bool isimage = false;
	float rotation = 0.0f;
	float sizebegin, sizeend;
	float lifetime = 1.0f;
	float liferemaining = 0.0f;
	bool active = false;
};


struct ParticleSystem {
	ParticleSystem();

	void OnUpdate(float ts);
	void OnRender(const glm::mat4& cammatrix);

	void Emit(const ParticleProps& pprops);

	std::vector<Particle> m_particlepool;
	uint32_t m_poolindex = 999;

};
