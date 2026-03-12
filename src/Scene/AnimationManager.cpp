#include "AnimationManager.h"


std::vector<AnimationOperation> AnimationManager::m_Operations;

glm::vec3 Interpolate(glm::vec3 start, glm::vec3 end, float t) {
	return (start + (end - start)*t);
}

float Interpolate(float a, float b, float t) {
	return (a + (b - a) * t);
}

// for animationoperations


AnimationOperation::AnimationOperation(std::vector<glm::vec3>& m_PointsPosition, float completetime, AnimationType atype, std::shared_ptr<Entity> e) {
	this->m_PointsPosition = m_PointsPosition;
	this->animationcompletetime = completetime;
	this->m_AnimationType = atype;
	this->m_Entity = e;
}
AnimationOperation::AnimationOperation(float initial_opacity, float final_opacity, float completetime, AnimationType atype, std::shared_ptr<Entity> e) {
	this->initial_opacity = initial_opacity;
	this->final_opacity = final_opacity;
	this->animationcompletetime = completetime;
	this->m_AnimationType = atype;
	this->m_Entity = e;
}



// for animation manager



void AnimationManager::AddOperation(AnimationOperation& op) {
	m_Operations.push_back(op);
}


void AnimationManager::Update(float ts) {


	for (auto &animationop : m_Operations) {

		// for position interpolation
		if (animationop.m_AnimationType == POSITION_INTERPOLATE) {
			unsigned int interpolationcount = animationop.m_PointsPosition.size() - 1;

			float single_interpolation_time = animationop.animationcompletetime / interpolationcount;

			unsigned int index = (unsigned int)(animationop.time / single_interpolation_time);

			if (index >= animationop.m_PointsPosition.size()-1)
				continue;

			float ttime = animationop.time - (single_interpolation_time * index);
			glm::vec3 iposition = Interpolate(animationop.m_PointsPosition[index], animationop.m_PointsPosition[index + 1], ttime / single_interpolation_time);
		
			auto& tc = animationop.m_Entity->GetComponent<TransformComponent>();
			tc.position = iposition;

			animationop.time += ts;
		}

		// for opacity interpolation
		else if (animationop.m_AnimationType == OPACITY_INTERPOLATE) {

			
			float to = Interpolate(animationop.initial_opacity, animationop.final_opacity, animationop.time/animationop.animationcompletetime);

			if (animationop.m_Entity->HasComponent<SpriteRendererComponent>()) {
				auto& spc = animationop.m_Entity->GetComponent<SpriteRendererComponent>();

				if (spc.type == TEXTURE_SPRITERENDERER) {
					spc.opacity = to;
				}
				else if (spc.type == COLOR_SPRITERENDERER) {
					spc.color.a = to;
				}
			}
 			
			else if (animationop.m_Entity->HasComponent<FontRendererComponent>()) {
				auto& fc = animationop.m_Entity->GetComponent<FontRendererComponent>();

				fc.opacity = to;
			}
			animationop.time += ts;
		}
	}

	for (unsigned int i = 0; i < m_Operations.size(); i++) {
		if (m_Operations[i].time >= m_Operations[i].animationcompletetime) {
			RemoveOperation(i);
			i--;
		}
	}
}


void AnimationManager::RemoveOperation(unsigned int index) {
	m_Operations.erase(m_Operations.begin() + index);
}


void AnimationManager::RemoveOperation(std::shared_ptr<Entity> e) {

	unsigned int len = m_Operations.size();
	for (unsigned int i = 0; i < len; i++) {
		
		if (m_Operations[i].m_AnimationType == POSITION_INTERPOLATE) {
			auto& tc = e->GetComponent<TransformComponent>();
			unsigned int plength = m_Operations[i].m_PointsPosition.size();

			glm::vec3 position = m_Operations[i].m_PointsPosition[plength - 1];

			tc.position = position;

			m_Operations[i].time = m_Operations[i].animationcompletetime;
		}
	}
}