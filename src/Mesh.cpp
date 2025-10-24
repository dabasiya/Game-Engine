#include <glad/glad.h>
#include "Mesh.h"

#include <iostream>

Mesh::Mesh(std::vector<ModelVertex> a_vertices, std::vector<unsigned int> a_indices, std::vector<std::shared_ptr<Texture>> texture) {
	size = a_indices.size();

	if (texture.size() > 0)
		hastexture = true;

	vbo = VBO(a_vertices.size() * sizeof(ModelVertex), a_vertices.data(), GL_STATIC_DRAW);

	// position x, y, z
	vao.LinkVBO(vbo, 0, 3, 26 * sizeof(float), 0);
	// normals x, y, z
	vao.LinkVBO(vbo, 1, 3, 26 * sizeof(float), (void*)(3 * sizeof(float)));
	// texturecoords u, v
	vao.LinkVBO(vbo, 2, 2, 26 * sizeof(float), (void*)(6 * sizeof(float)));
	// tangent x, y, z
	vao.LinkVBO(vbo, 3, 3, 26 * sizeof(float), (void*)(8 * sizeof(float)));
	// bitangent x, y, z
	vao.LinkVBO(vbo, 4, 3, 26 * sizeof(float), (void*)(11 * sizeof(float)));
	// color r, g, b, a
	vao.LinkVBO(vbo, 5, 4, 26 * sizeof(float), (void*)(14 * sizeof(float)));
	// bone ids
	vao.LinkVBO(vbo, 6, 4, 26 * sizeof(float), (void*)(18 * sizeof(float)));
	// bone weights
	vao.LinkVBO(vbo, 7, 4, 26 * sizeof(float), (void*)(22 * sizeof(float)));


	ebo = EBO(a_indices.size() * sizeof(unsigned int), a_indices.data());

	textures = texture;
}

void Mesh::Draw(Shader& shader) {
	//shader.Bind();
	unsigned int diffusenr = 1;
	unsigned int normalnr = 1;
	unsigned int specularnr = 1;
	unsigned int heightnr = 1;
	vao.Bind();
	ebo.Bind();

	for (unsigned int i = 0; i < textures.size(); i++) {

		std::string number;
		std::string name = textures[i]->type;


		if (name == "texture_diffuse")
			number = std::to_string(diffusenr++);
		else if (name == "texture_specular")
			number = std::to_string(specularnr++);
		else if (name == "texture_normalnr")
			number = std::to_string(normalnr++);
		else if (name == "texture_heightnr")
			number = std::to_string(heightnr++);

		name += number;

		textures[i]->Bind(shader, name, i);
	}
	shader.SetInt("hastexture", hastexture);
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);
}