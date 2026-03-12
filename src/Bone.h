#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/scene.h>

#include <AssimpHelper.h>

#include <string>

#include <vector>

struct KeyPosition {
	glm::vec3 position;
	float timestamp;
};

struct KeyRotation {
	glm::quat orientation;
	float timestamp;
};

struct KeyScale {
	glm::vec3 scale;
	float timestamp;
};

class Bone {
private:
	std::vector<KeyPosition> m_Positions;
	std::vector<KeyRotation> m_Rotation;
	std::vector<KeyScale> m_Scale;
	int m_NumPositions;
	int m_NumRotations;
	int m_NumScales;

	glm::mat4 m_LocalTransform;
	std::string m_Name;
	int m_ID;

public:
	Bone(const std::string& name, int ID, const aiNodeAnim* channel)
		:
		m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
	{
		m_NumPositions = channel->mNumPositionKeys;

		for (int positionindex = 0; positionindex < m_NumPositions; positionindex++) {
			aiVector3D aiPosition = channel->mPositionKeys[positionindex].mValue;
			float timestamp = channel->mPositionKeys[positionindex].mTime;
			KeyPosition data;
			data.position = AssimpHelper::ConvertVec3ToGLM(aiPosition);
			data.timestamp = timestamp;
			m_Positions.push_back(data);
		}

		m_NumRotations = channel->mNumRotationKeys;

		for (int rotationindex = 0; rotationindex < m_NumRotations; rotationindex++) {
			aiQuaternion aiquat = channel->mRotationKeys[rotationindex].mValue;
			float timestamp = channel->mRotationKeys[rotationindex].mTime;
			KeyRotation data;
			data.orientation = AssimpHelper::ConvertQuatToGLM(aiquat);
			data.timestamp = timestamp;
			m_Rotation.push_back(data);
		}

		m_NumScales = channel->mNumScalingKeys;

		for (int scaleindex = 0; scaleindex < m_NumScales; scaleindex++) {
			aiVector3D aiscale = channel->mScalingKeys[scaleindex].mValue;
			float timestamp = channel->mScalingKeys[scaleindex].mTime;
			KeyScale data;
			data.scale = AssimpHelper::ConvertVec3ToGLM(aiscale);
			data.timestamp = timestamp;
			m_Scale.push_back(data);
		}
	}

	void Update(float animationtime) {
		glm::mat4 scale = InterpolateScale(animationtime);
		glm::mat4 rotation = InterpolateRotation(animationtime);
		glm::mat4 position = InterpolatePosition(animationtime);
		m_LocalTransform = position * rotation * scale;
	}

	glm::mat4 GetLocalTransform() { return m_LocalTransform; }
	std::string GetBoneName() const { return m_Name; }
	int GetBoneID() { return m_ID; }

	int GetPositionIndex(float time) {
		for (int index = 0; index < m_NumPositions - 1; index++) {
			if (time < m_Positions[index + 1].timestamp)
				return index;
		}
	}

	int GetRotationIndex(float time) {
		for (int index = 0; index < m_NumRotations - 1; index++) {
			if (time < m_Rotation[index + 1].timestamp)
				return index;
		}
	}

	int GetScaleIndex(float time) {
		for (int index = 0; index < m_NumScales - 1; index++) {
			if (time < m_Scale[index + 1].timestamp)
				return index;
		}
	}

private:
	float GetScaleFactor(float lasttimestamp, float nexttimestamp, float animationtime) {
		float scalefactor = 0.0f;
		float midwaylength = animationtime - lasttimestamp;
		float framediff = nexttimestamp - lasttimestamp;
		scalefactor = midwaylength / framediff;
		return scalefactor;
	}

	glm::mat4 InterpolatePosition(float animationtime) {
		if (m_NumPositions == 1)
			return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

		int p0Index = GetPositionIndex(animationtime);
		int p1Index = p0Index + 1;
		float scalefactor = GetScaleFactor(m_Positions[p0Index].timestamp, m_Positions[p1Index].timestamp, animationtime);
		glm::vec3 finalposition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scalefactor);
		return glm::translate(glm::mat4(1.0f), finalposition);
	}

	glm::mat4 InterpolateRotation(float animationtime) {
		if (m_NumRotations == 1) {
			auto rotation = glm::normalize(m_Rotation[0].orientation);
			return glm::toMat4(rotation);
		}

		int p0Index = GetRotationIndex(animationtime);
		int p1Index = p0Index + 1;
		float scalefactor = GetScaleFactor(m_Rotation[p0Index].timestamp, m_Rotation[p1Index].timestamp, animationtime);
		glm::quat finalquat = glm::slerp(m_Rotation[p0Index].orientation, m_Rotation[p1Index].orientation, scalefactor);
		finalquat = glm::normalize(finalquat);
		return glm::toMat4(finalquat);
	}

	glm::mat4 InterpolateScale(float animationtime) {
		if (m_NumScales == 1)
			return glm::scale(glm::mat4(1.0f), m_Scale[0].scale);

		int p0Index = GetScaleIndex(animationtime);
		int p1Index = p0Index + 1;
		float scalefactor = GetScaleFactor(m_Scale[p0Index].timestamp, m_Scale[p1Index].timestamp, animationtime);
		glm::vec3 finalscale = glm::mix(m_Scale[p0Index].scale, m_Scale[p1Index].scale, scalefactor);
		return glm::scale(glm::mat4(1.0f), finalscale);
	}
};