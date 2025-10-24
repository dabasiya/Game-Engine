#pragma once


#include <Scene/Components.h>

enum AnimationType {
	POSITION_INTERPOLATE,
	OPACITY_INTERPOLATE
};


struct AnimationOperation {
	float time = 0.0f;

	std::shared_ptr<Entity> m_Entity;

	AnimationType m_AnimationType;
	
	float animationcompletetime;
	// points if position interpolation is used
	std::vector<glm::vec3> m_PointsPosition;

	// opacity if opacity interpolation is used
	float initial_opacity, final_opacity;

	AnimationOperation(std::vector<glm::vec3> &m_PointsPosition, float completetime, AnimationType atype, std::shared_ptr<Entity> e);
	AnimationOperation(float initial_opacity, float final_opacity, float completetime, AnimationType atype, std::shared_ptr<Entity> e);
};


struct AnimationManager {

	static std::vector<AnimationOperation> m_Operations;

	static void AddOperation(AnimationOperation& op);
	static void Update(float ts);
	static void RemoveOperation(unsigned int index);
};