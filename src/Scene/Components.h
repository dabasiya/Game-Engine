#pragma once


#include <SubTexture.h>

#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <OrthographicCamera.h>
#include <Scripts/EntityScript.h>
#include <Scripts/ScriptManager.h>
#include <Animation.h>
#include <Scene/AnimationUtil.h>
#include <ParticleSystem.h>

struct NameComponent {
	std::string name;

        bool active = true;

	NameComponent() {}
	NameComponent(const std::string& a_name)
		: name(a_name)
	{

	}
};


struct TransformComponent {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	TransformComponent() {}
	TransformComponent(const glm::vec3& value)
		: position(value)
	{
	}

	glm::mat4 getmatrix() {
		glm::mat4 rotationmatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		return glm::translate(glm::mat4(1.0f), position) *
			rotationmatrix *
			glm::scale(glm::mat4(1.0f), scale);
	}
};


struct CameraComponent {
	OrthographicCamera camera;

	bool Primary = false;

        float x_min = 0.0f;
        float x_max = 0.0f;

        float y_min = 0.0f;
        float y_max = 0.0f;

	CameraComponent() {}
	CameraComponent(const OrthographicCamera& a_camera, bool a_primary)
		: camera(a_camera), Primary(a_primary) {

	}
};

enum {
	COLOR_SPRITERENDERER,
	TEXTURE_SPRITERENDERER
};

struct SpriteRendererComponent {

	SubTexture m_subtexture;

	bool type = 0; // by default its color type spriterenderercomponent

	bool transparent = false;
	glm::vec4 color = glm::vec4(0.0f);

	SpriteRendererComponent(const glm::vec4& a_color)
		: color(a_color), type(0)
	{

	}

	SpriteRendererComponent(const SubTexture& a_subtexture)
		: m_subtexture(a_subtexture), type(1)
	{
	}

	SpriteRendererComponent()
		: type(0), color(glm::vec4(1.0f))
	{
	}
};

struct FontRendererComponent {
	std::string text;

	float opacity;

	float pixelsize;

	FontRendererComponent(const std::string& a_text, float psize)
		: text(a_text), opacity(1.0f), pixelsize(psize)
	{}
};

struct ScriptComponent {
	EntityScript* script;

	std::string scriptname;

	ScriptComponent(const std::string& name)
		: scriptname(name), script(nullptr)
	{}

	void instantiate() {
		script = scriptmanager::createscript(scriptname);
	}

	void destroy() {
		if (script != nullptr) {
			delete script;
			script = nullptr;
		}
	}

};

// animation group

struct AnimationGroupComponent {
	std::unordered_map<std::string, Animation> animations;

	std::vector<std::pair<std::string, std::string>> animationpath;

	void addanimationpath(const std::string& key, const std::string& filepath) {
		animationpath.push_back(std::make_pair(key, filepath));
	}

	void loadanimations() {
		animations.clear();

		for (const auto& pair : animationpath) {
			Animation a = readanimation(pair.second);

			animations[pair.first] = a;
		}
	}

};


// rigidbody
struct RigidBody2DComponent {

	bool fixedrotation = false;
	b2BodyType type = b2BodyType::b2_staticBody;
	b2Body* body = nullptr;
	// for collision and etc in games
	unsigned int id;

	RigidBody2DComponent() = default;
	RigidBody2DComponent(const RigidBody2DComponent&) = default;
};


// box collider
struct BoxCollider2DComponent {
	glm::vec2 offset = { 0.0f, 0.0f };
	glm::vec2 size = { 0.5f, 0.5f };

	float density = 1.0f;
	float friction = 0.4f;

	float width = 0.0f;
	float height = 0.0f;

	unsigned int objectid = 0;

	bool isSensor = false;

	b2Fixture* fixture;

	BoxCollider2DComponent() = default;
	BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
};

// chain shape collider for map collision and etc
struct ChainShapeColliderComponent {
	std::vector<b2Vec2> vertices;

	b2Fixture* fixture;

	unsigned int objectid = 0;

	ChainShapeColliderComponent() = default;
};


struct ParticleGeneratorComponent {
	ParticleProps m_particleprops;

	float time = 0.0f;

	// count of emit particle per frame
	unsigned int count;

        // is active or not
        bool active = true;
};
