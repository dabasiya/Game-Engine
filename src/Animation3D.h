#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "Bone.h"

#include "Model.h"


#include <iostream>


struct AssimpNodeData {
	glm::mat4 transformation;
	std::string name;
	int childrencount;
	std::vector<AssimpNodeData> childern;
};


struct RuntimeBoneData {
	std::string name;
	glm::mat4 LocalTransform;
	glm::mat4 FinalAnimatedMatrix;
	glm::mat4 OffsetMatrix;
	int ParentIndex;
};

class Animation3D {
public:
	Animation3D() = default;

	static std::string GetAnimationName(const std::string& animationpath, int index) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationpath, aiProcess_Triangulate);
		unsigned int animationcount = scene->mNumAnimations;

		if (index >= animationcount)
			return std::string("");

		auto animation = scene->mAnimations[index];

		return std::string(animation->mName.C_Str());
	}

	Animation3D(const std::string& animationpath, Model* model, unsigned int animationindex = 0) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationpath, aiProcess_Triangulate);
		auto animation = scene->mAnimations[animationindex];
		m_Duration = animation->mDuration;
		m_TickPerSecond = animation->mTicksPerSecond;
		
		std::cout << "Before ReadHierarchyData: " << m_RuntimeBones.size() << std::endl;
		ReadHierarchyData(m_RootNode, scene->mRootNode);
		std::cout << "After ReadHierarchyData: " << m_RuntimeBones.size() << std::endl;
		ReadMissingBone(animation, *model);
		std::cout << "After ReadMissingBone: " << m_RuntimeBones.size() << std::endl;
		int ind = 0;
		m_RuntimeBones.reserve(m_Bones.size());

		RuntimeBoneData data;
		int size = m_Bones.size() + 1;
		for (unsigned int i = 0; i < size; i++) {
			m_RuntimeBones.push_back(data);
		}
		PopulateRuntimeBones(&m_RootNode, -1, ind);
		std::cout << "After PopulateRuntimeBones: " << m_RuntimeBones.size() << std::endl;
	}

	Bone* FindBone(const std::string& name) {
		auto itr = std::find_if(m_Bones.begin(), m_Bones.end(), [&](const Bone& bone) {
			return bone.GetBoneName() == name;
			});
		if (itr == m_Bones.end()) return nullptr;
		return &(*itr);
	}

	int GetTickPerSecond() { return m_TickPerSecond; }
	float GetDuration() { return m_Duration; }
	AssimpNodeData& GetRootNode() { return m_RootNode; }
	std::map<std::string, BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }

	void ReadMissingBone(const aiAnimation* animation, Model& model) {
		int size = animation->mNumChannels;

		auto& boneInfoMap = model.GetBoneInfoMap();
		int& bonecount = model.GetBoneCount();

		for (int i = 0; i < size; i++) {
			auto channel = animation->mChannels[i];
			std::string bonename = channel->mNodeName.data;

			if (boneInfoMap.find(bonename) == boneInfoMap.end()) {
				boneInfoMap[bonename].id = bonecount;
				bonecount++;
			}
			m_Bones.push_back(Bone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].id, channel));
		}
		m_BoneInfoMap = boneInfoMap;
	}


	void ReadHierarchyData(AssimpNodeData& dest, const aiNode* node) {
		dest.name = node->mName.data;
		dest.transformation = AssimpHelper::ConvertMatrixToGLM(node->mTransformation);
		dest.childrencount = node->mNumChildren;

		for (int i = 0; i < node->mNumChildren; i++) {
			AssimpNodeData data;
			ReadHierarchyData(data, node->mChildren[i]);
			dest.childern.push_back(data);
		}
	}


	// Note: We need a new vector to hold the final, correctly ordered data.
// It's cleaner to build the final, sorted vector in a separate function.

	void PopulateRuntimeBones(AssimpNodeData* data, int parent_flat_index, int& current_flat_index) {

		int current_node_flat_index = current_flat_index;

		if (current_flat_index > m_Bones.size()) {
			std::cout << current_flat_index << std::endl;
			//return;
		}
		
		current_flat_index++;


		m_RuntimeBones[current_node_flat_index].ParentIndex = parent_flat_index;
		m_RuntimeBones[current_node_flat_index].LocalTransform = data->transformation;
		m_RuntimeBones[current_node_flat_index].FinalAnimatedMatrix = data->transformation;
		m_RuntimeBones[current_node_flat_index].name = data->name;
		
		m_RuntimeBones[current_node_flat_index].OffsetMatrix = m_BoneInfoMap[data->name].offset;

		
		for (auto& cdata : data->childern) {
			PopulateRuntimeBones(&cdata, current_node_flat_index, current_flat_index);
		}
	}

	float m_Duration;
	int m_TickPerSecond;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::map<std::string, BoneInfo> m_BoneInfoMap;
	std::vector<RuntimeBoneData> m_RuntimeBones;
};