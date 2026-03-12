#include "Model.h"

#include <AssimpHelper.h>
#include <iostream>


Model::Model(const char* path) {

	loadmodel(path);
}

Model::Model(const std::string& path) {
	loadmodel(path.c_str());
}

void Model::loadmodel(const char* path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << importer.GetErrorString() << std::endl;
		return;
	}

	processnode(scene->mRootNode, scene);
}

void Model::processnode(aiNode* node, const aiScene* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processmesh(mesh, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processnode(node->mChildren[i], scene);
	}
}

Mesh Model::processmesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<ModelVertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<std::shared_ptr<Texture>> textures;


	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		ModelVertex vertex;

		SetVertexBoneDataToDefault(vertex);


		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		if (mesh->HasNormals()) {
			vertex.normal.x = mesh->mNormals[i].x;
			vertex.normal.y = mesh->mNormals[i].y;
			vertex.normal.z = mesh->mNormals[i].z;
		}

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		if (material) {
			aiColor4D color;
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
				glm::vec4 acolor;
				acolor.r = color.r;
				acolor.g = color.g;
				acolor.b = color.b;
				acolor.a = color.a;
				vertex.color = acolor;
			}
		}

		if (mesh->mTextureCoords[0]) {
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;

			vertex.texcoords = vec;

			vertex.tangent.x = mesh->mTangents[i].x;
			vertex.tangent.y = mesh->mTangents[i].y;
			vertex.tangent.z = mesh->mTangents[i].z;

			vertex.bitangent.x = mesh->mBitangents[i].x;
			vertex.bitangent.y = mesh->mBitangents[i].y;
			vertex.bitangent.z = mesh->mBitangents[i].z;
		}

		else {
			vertex.texcoords = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

	auto diffuset = loadmaterialtextures(mat, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuset.begin(), diffuset.end());

	auto speculart = loadmaterialtextures(mat, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), speculart.begin(), speculart.end());

	auto normalt = loadmaterialtextures(mat, aiTextureType_NORMALS, "texture_normal");
	textures.insert(textures.end(), normalt.begin(), normalt.end());

	auto height = loadmaterialtextures(mat, aiTextureType_HEIGHT, "texture_height");
	textures.insert(textures.end(), height.begin(), height.end());

	ExtractBoneWeightsForVertices(vertices, mesh, scene);
	return Mesh(vertices, indices, textures);
}

void Model::SetVertexBoneDataToDefault(ModelVertex& vertex) {
	for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
		vertex.m_BoneIDs[i] = -1.0f;
		vertex.m_Weights[i] = 0.0f;
	}
}

void Model::Draw(Shader& m_shader) {
	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(m_shader);
	}
}

std::vector<std::shared_ptr<Texture>> Model::loadmaterialtextures(aiMaterial* mt, aiTextureType type, std::string type_name) {
	std::vector < std::shared_ptr<Texture>> textures;

	for (unsigned int i = 0; i < mt->GetTextureCount(type); i++) {
		aiString str;
		mt->GetTexture(type, i, &str);
		bool skip = false;

		for (unsigned int j = 0; j < loaded_textures.size(); j++) {
			if (strcmp(loaded_textures[j]->path.c_str(), str.C_Str()) == 0) {
				textures.push_back(loaded_textures[j]);
				skip = true;
				break;
			}
		}

		if (!skip) {
			std::shared_ptr<Texture> texture1 = std::make_shared<Texture>(str.C_Str(), type_name);
			textures.push_back(texture1);
			loaded_textures.push_back(texture1);
		}
	}

	return textures;
}

void Model::SerVertexBoneData(ModelVertex& vertex, float boneid, float weight) {
	for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
		if (vertex.m_BoneIDs[i] < 0.0f) {
			vertex.m_BoneIDs[i] = boneid;
			vertex.m_Weights[i] = weight;
			break;
		}
	}
}

void Model::ExtractBoneWeightsForVertices(std::vector<ModelVertex>& vertices, aiMesh* mesh, const aiScene* scene) {
	int bonecount = mesh->mNumBones;

	for (unsigned int i = 0; i < bonecount; i++) {
		float boneid = -1.0f;
		std::string bonename = mesh->mBones[i]->mName.C_Str();

		if (m_BoneInfoMap.find(bonename) == m_BoneInfoMap.end()) {
			BoneInfo newbone;
			newbone.id = m_BoneCounter;
			newbone.offset = AssimpHelper::ConvertMatrixToGLM(mesh->mBones[i]->mOffsetMatrix);
			m_BoneInfoMap[bonename] = newbone;
			boneid = (float)m_BoneCounter;
			m_BoneCounter++;
		}
		else {
			boneid = (float)m_BoneInfoMap[bonename].id;
		}

		auto weights = mesh->mBones[i]->mWeights;
		int weightcount = mesh->mBones[i]->mNumWeights;

		for (unsigned int j = 0; j < weightcount; j++) {
			int vertexid = weights[j].mVertexId;
			float weight = weights[j].mWeight;
			SerVertexBoneData(vertices[vertexid], boneid, weight);
		}
	}
}