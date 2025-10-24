#pragma once

#include <glm/glm.hpp>
#include <VBO.h>
#include <VAO.h>
#include <EBO.h>
#include <Shader.h>
#include <Texture.h>
#include <vector>

#define MAX_BONE_INFLUENCE 4

struct ModelVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoords;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	glm::vec4 color;
	float m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];
};


class Mesh {
public:
	Mesh(std::vector<ModelVertex> a_vertices, std::vector<unsigned int> a_indices, std::vector<std::shared_ptr<Texture>> textures);
	void Draw(Shader& shader);

private:

	bool hastexture = false;

	VBO vbo;
	VAO vao;
	EBO ebo;

	std::vector<std::shared_ptr<Texture>> textures;

	unsigned int size = 0;
};