
#pragma once

#include <Shader.h>
#include <vector>

#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <map>

struct BoneInfo {
	int id;

	// for rotate bone vertices
	glm::mat4 offset;
};


class Model {
public:
	Model(const char* path);

	Model(const std::string& path);

	void Draw(Shader& m_shader);

	auto& GetBoneInfoMap() { return m_BoneInfoMap; }
	int& GetBoneCount() { return m_BoneCounter; }

private:

	std::map<std::string, BoneInfo> m_BoneInfoMap;
	int m_BoneCounter = 0;

	void SetVertexBoneDataToDefault(ModelVertex& vertex);

	std::vector<Mesh> meshes;
	std::vector<std::shared_ptr<Texture>> loaded_textures;

	void loadmodel(const char* path);
	void processnode(aiNode* node, const aiScene* scene);
	Mesh processmesh(aiMesh* mesh, const aiScene* scene);

	void SerVertexBoneData(ModelVertex& vertex, float boneid, float weight);

	void ExtractBoneWeightsForVertices(std::vector<ModelVertex>& vertices, aiMesh* mesh, const aiScene* scene);

	std::vector<std::shared_ptr<Texture>> loadmaterialtextures(aiMaterial* mt, aiTextureType type, std::string type_name);
};