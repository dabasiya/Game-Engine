#include "ParticleSystem.h"
#include "Random.h"
#include "Renderer2D.h"
#include <glm/gtx/compatibility.hpp>

#include <glad/glad.h>

ParticleSystem::ParticleSystem() {
	m_particlepool.resize(1000);
}

void ParticleSystem::OnUpdate(float ts) {
	for (auto& particle : m_particlepool) {
		if (!particle.active)
			continue;

		else if (particle.liferemaining < 0.0f) {
			particle.liferemaining = 0.0f;
			particle.active = false;
			continue;
		}

		particle.liferemaining -= ts;
		particle.position.x += particle.velocity.x;
		particle.position.y += particle.velocity.y;
		particle.rotation += glm::degrees(ts * 3);
	}
}

void ParticleSystem::OnRender(const glm::mat4& cammatrix) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	Renderer2D::BeginScene(cammatrix);

	for (auto& particle : m_particlepool) {
		if (!particle.active)
			continue;

		float life = particle.liferemaining / particle.lifetime;
		glm::vec4 color = glm::lerp(particle.colorend, particle.colorbegin, life);

		float size = glm::lerp(particle.sizeend, particle.sizebegin, life);

		if (!particle.isimage)
			Renderer2D::DrawRotatedQuad(particle.position, { size, size }, particle.rotation, color);
		else
			Renderer2D::DrawRotatedQuad(particle.position, { size, size }, particle.rotation, particle.m_subtexture, color);
	}

	Renderer2D::EndScene();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleSystem::Emit(const ParticleProps& pprops) {
	Particle& particle = m_particlepool[m_poolindex];

	particle.active = true;
	particle.colorbegin = pprops.colorbegin;
	particle.colorend = pprops.colorend;
	particle.sizebegin = pprops.sizebegin;
	particle.sizeend = pprops.sizeend;
	particle.lifetime = pprops.lifetime;
	particle.liferemaining = pprops.lifetime;
	particle.velocity = pprops.velocity;
	particle.velocity.x += pprops.velocityvariation.x * (Random::Float() - 0.5f);
	particle.velocity.y += pprops.velocityvariation.y * (Random::Float() - 0.5f);
	particle.sizebegin += pprops.sizevariation * (Random::Float() - 0.5f);
	particle.rotation = Random::Float() * 2.0f * glm::pi<float>();

        if(pprops.generate_from_area) {
                float x = pprops.scale.x * (Random::Float() - 0.5f);
                float y = pprops.scale.y * (Random::Float() - 0.5f);

                glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(pprops.z_rotation), glm::vec3(0.0f, 0.0f, 1.0f));

                glm::vec4 newvector = rotation * glm::vec4(x, y, 0.0f, 0.0f);
                particle.position = pprops.position + glm::vec3(newvector);
        }
        else {
                particle.position = pprops.position;
        }
        particle.m_subtexture = pprops.m_subtexture;
	particle.isimage = pprops.isimage;

	m_poolindex = --m_poolindex % 1000;
}
