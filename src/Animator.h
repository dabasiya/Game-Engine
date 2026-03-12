#pragma once

#include "Animation3D.h"
#include <gladr.h>

#include <iostream>

class Animator {
public:

	Animator(Animation3D* currentanimation) {
		m_CurrentTime = 0.0f;
		m_CurrentAnimation = currentanimation;

		m_FinalBoneMetrices.reserve(223);

		for (int i = 0; i < 223; i++)
			m_FinalBoneMetrices.push_back(glm::mat4(1.0f));
	}

	void UpdateAnimation(float animationtime) {

		m_DeltaTime = animationtime;
		if (m_CurrentAnimation) {
			m_CurrentTime += m_CurrentAnimation->GetTickPerSecond() * animationtime;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			//CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
			CalculateBoneTransform(m_CurrentAnimation->m_RuntimeBones, glm::mat4(1.0f));
		}
	}

	void CalculateBoneTransform(std::vector<RuntimeBoneData>& m_runtimebones, glm::mat4 parenttransform) {

		unsigned int size = m_runtimebones.size();

		auto& boneinfomap = m_CurrentAnimation->GetBoneInfoMap();

		for (unsigned int i = 0; i < size; i++) {

			std::string nodename = m_runtimebones[i].name;
			glm::mat4 nodetransform = m_runtimebones[i].LocalTransform;

			Bone* bone = m_CurrentAnimation->FindBone(nodename);
			if (bone) {
				bone->Update(m_CurrentTime);
				nodetransform = bone->GetLocalTransform();
			}

			glm::mat4 global = nodetransform;

			int parind = m_runtimebones[i].ParentIndex;

			if (parind < -1)
				continue;

			if (m_runtimebones[i].ParentIndex != -1) {
				global = m_runtimebones[parind].FinalAnimatedMatrix * global;
			}

			glm::mat4 offset = boneinfomap[nodename].offset;
			m_runtimebones[i].FinalAnimatedMatrix = global;
			int boneIndex = boneinfomap[nodename].id;
			if (boneIndex >= 0 && boneIndex < m_FinalBoneMetrices.size()) {
				m_FinalBoneMetrices[boneIndex] = global * offset;
			}
		}
	}

	
	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parenttransform) {
		std::string nodename = node->name;
		glm::mat4 nodetransform = node->transformation;

		Bone* bone = m_CurrentAnimation->FindBone(nodename);
		if (bone) {
			bone->Update(m_CurrentTime);
			nodetransform = bone->GetLocalTransform();
		}

		glm::mat4 globaltransformation = parenttransform * nodetransform;

		auto& boneinfomap = m_CurrentAnimation->GetBoneInfoMap();


		if (boneinfomap.find(nodename) != boneinfomap.end()) {
			int index = boneinfomap[nodename].id;
			glm::mat4 offset = boneinfomap[nodename].offset;
			m_FinalBoneMetrices[index] = globaltransformation * offset;
		}

		for (int i = 0; i < node->childrencount; i++) {
			CalculateBoneTransform(&node->childern[i], globaltransformation);
		}

		
	}

	

	std::vector<glm::mat4>& GetFinalBoneMetrices() {
		return m_FinalBoneMetrices;
	}

	void Reset() {
		m_CurrentTime = 0.0f;
	}

private:
	std::vector<glm::mat4> m_FinalBoneMetrices;
	Animation3D* m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;

	bool isFirsttime = true;

	unsigned int UBO;
};