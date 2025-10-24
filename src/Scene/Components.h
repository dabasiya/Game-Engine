#pragma once


#include <SubTexture.h>
#include <glm/gtx/euler_angles.hpp>
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <OrthographicCamera.h>
#include <PerspectiveCamera.h>
#include <Scripts/EntityScript.h>
#include <Scripts/ScriptManager.h>
#include <Animation.h>
#include <Scene/AnimationUtil.h>
#include <ParticleSystem.h>
#include <Model.h>

#include <FrameBuffer.h>

#include <btBulletDynamicsCommon.h>

enum CameraType {
	Orthographic = 0,
	Perspective = 1
};

enum LightType {
	DIRECTIONAL_LIGHT = 0,
	SPOT_LIGHT = 1,
	POINT_LIGHT = 2
};

struct NameComponent {
	std::string name;

	bool active = true;

	NameComponent() {}
	NameComponent(const std::string& a_name)
		: name(a_name)
	{

	}
};

struct RelationshipComponent {

	std::shared_ptr<Entity> parentEntity = nullptr;

	std::vector<std::shared_ptr<Entity>> childEntities;

	bool isUIOpen = false;

	RelationshipComponent() {}
};


struct TransformComponent {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	glm::mat4 worldtransform = glm::mat4(1.0f);

	TransformComponent() {}
	TransformComponent(const glm::vec3& value)
		: position(value)
	{
	}

	glm::mat4 getmatrix() {
		glm::mat4 rotationmatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));

		return glm::translate(glm::mat4(1.0f), position) *
			rotationmatrix *
			glm::scale(glm::mat4(1.0f), scale);
	}
};


struct CameraComponent {
	OrthographicCamera camerao;
	PerspectiveCamera camerap;

	CameraType cameratype = CameraType::Orthographic;

	bool Primary = false;

	float x_min = 0.0f;
	float x_max = 0.0f;

	float y_min = 0.0f;
	float y_max = 0.0f;

	CameraComponent() {}
	CameraComponent(const OrthographicCamera& a_camera, bool a_primary)
		: camerao(a_camera), Primary(a_primary) {

	}
};

enum {
	COLOR_SPRITERENDERER,
	TEXTURE_SPRITERENDERER
};

struct SpriteRendererComponent {

	SubTexture m_subtexture;

	bool type = 0; // by default its color type spriterenderercomponent


	float opacity = 1.0f;


	bool transparent = false;
	glm::vec4 color = glm::vec4(0.0f);

	SpriteRendererComponent(const glm::vec4& a_color)
		: color(a_color), type(0)
	{

	}

	SpriteRendererComponent(const SubTexture& a_subtexture, const glm::vec4& a_color, bool type)
		: m_subtexture(a_subtexture), color(a_color), type(type) {

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

	FontRendererComponent() = default;

	FontRendererComponent(const std::string& a_text, float psize, float opacity = 1.0f)
		: text(a_text), opacity(opacity), pixelsize(psize)
	{}
};

struct ScriptComponent {
	EntityScript* script;

	std::string scriptname;

	ScriptComponent() = default;

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

	RigidBody2DComponent(bool fixedrotation, b2BodyType type, unsigned int id)
		: fixedrotation(fixedrotation), type(type), id(id) {}
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

	bool sameasscale = false;
	bool isSensor = false;

	b2Fixture* fixture;

	BoxCollider2DComponent() = default;
	BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	BoxCollider2DComponent(float density, float friction, float width, float height, unsigned int objectid, bool sameasscale, bool isSensor)
		: density(density), friction(friction), width(width), height(height), objectid(objectid), sameasscale(sameasscale), isSensor(isSensor) {}
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


struct Model3DComponent {

	std::string filepath;

	std::shared_ptr<Model> mModel = nullptr;

	std::string shadername;
};


struct LightComponent {
	glm::vec3 color;

	LightType lighttype;

	// In Case of DirectionLight and SpotLight
	glm::vec3 direction;

	// In Case of spotlight
	float angle;

	FrameBuffer m_FrameBuffer;
	std::shared_ptr<Texture> m_Texture;

	void Update() {
		
		if (m_Texture == nullptr) {
			if (lighttype == POINT_LIGHT)
				m_Texture = std::make_shared<Texture>(Window::Width, Window::Width, GL_DEPTH_COMPONENT24);
			else
				m_Texture = std::make_shared<Texture>(Window::Width, Window::Height, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE);
			m_FrameBuffer.AddDepthAttachment(*m_Texture, GL_DEPTH_ATTACHMENT);
		}
		else if (Window::Width != m_Texture->width && lighttype == POINT_LIGHT) {
			m_Texture.reset();
			m_Texture = std::make_shared<Texture>(Window::Width, Window::Width, GL_DEPTH_COMPONENT24);
			m_FrameBuffer.AddDepthAttachment(*m_Texture, GL_DEPTH_ATTACHMENT);
		}
		else if ((Window::Width != m_Texture->width || Window::Height != m_Texture->height) && lighttype != POINT_LIGHT) {
			m_Texture.reset();
			m_Texture = std::make_shared<Texture>(Window::Width, Window::Height, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE);
			m_FrameBuffer.AddDepthAttachment(*m_Texture, GL_DEPTH_ATTACHMENT);
		}
	}
};


enum CARDTYPE {
	WHITE = 0,
	GREEN, 
	RED, 
	YELLOW, 
	BLUE
};

struct CardComponent {
	CARDTYPE m_Cardtype = (CARDTYPE)0;
};


enum class PhysicsShapeType {
	Box, 
	Sphere,
	Capsule,
	Infinite_Plane
};

struct PhysicsComponent {
	
	// mass
	float Mass = 1.0f;

	// restitution
	float Restitution = 0.0f;

	float friction = 1.0f;

	PhysicsShapeType ShapeType = PhysicsShapeType::Box;

	// Box properties
	glm::vec3 BoxHalfExtents = glm::vec3(0.5f, 0.5f, 0.5f);

	// Sphere properties (if ShapeType == Sphere)
	float SphereRadius = 0.5f;

	// The starting position (saved from the editor world state)
	glm::vec3 InitialPosition = glm::vec3(0.0f);
	glm::quat InitialRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);



	// bullet components
	btRigidBody* m_rigidBody = nullptr;
	btDefaultMotionState* m_motionState = nullptr;
	btCollisionShape* m_collisionShape = nullptr;

	// constructor and methods

	PhysicsComponent() = default;

	void initializeRuntimeBody(btDiscreteDynamicsWorld* world) {
		if (m_rigidBody) {
			// Body already exists, likely an error, but we skip to be safe.
			return;
		}

		// 1. Create Collision Shape based on stored type (ShapeType)
		switch (ShapeType) {
		case PhysicsShapeType::Box:
			m_collisionShape = new btBoxShape(toBtVec(BoxHalfExtents));
			break;
		case PhysicsShapeType::Sphere:
			m_collisionShape = new btSphereShape(SphereRadius);
			break;
		case PhysicsShapeType::Capsule:
			m_collisionShape = new btCapsuleShape(SphereRadius, BoxHalfExtents.y); // Example
			break;
		case PhysicsShapeType::Infinite_Plane:
			m_collisionShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0.0f);
			break;
		default:
			// Handle error or default to a box
			m_collisionShape = new btBoxShape(btVector3(1, 1, 1));
			break;
		}

		// 2. Initial Transform is set from the stored InitialPosition/InitialRotation
		btTransform startTransform;
		startTransform.setIdentity();
		startTransform.setOrigin(toBtVec(InitialPosition));
		startTransform.setRotation(toBtQuat(InitialRotation));

		// 3. Calculate Inertia based on stored Mass
		btVector3 localInertia(0, 0, 0);
		if (Mass != 0.0f) {
			m_collisionShape->calculateLocalInertia(Mass, localInertia);
		}

		// 4. Create Motion State and Rigid Body
		m_motionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(
			Mass, m_motionState, m_collisionShape, localInertia
		);
		m_rigidBody = new btRigidBody(rbInfo);

		m_rigidBody->setRestitution(Restitution);
		m_rigidBody->setFriction(friction);

		// 5. Add to the World
		world->addRigidBody(m_rigidBody);
	}

	// New function: Called to destroy the body and prepare for re-entry
	void destroyRuntimeBody(btDiscreteDynamicsWorld* world) {
		if (!m_rigidBody) {
			return; // Nothing to destroy
		}

		// 1. Remove the body from the world first
		world->removeRigidBody(m_rigidBody);

		// 2. Delete all allocated Bullet objects
		delete m_rigidBody;
		delete m_motionState;
		delete m_collisionShape;

		// 3. Clear the pointers (Crucial for safe re-entry into editor mode)
		m_rigidBody = nullptr;
		m_motionState = nullptr;
		m_collisionShape = nullptr;
	}

	void updateGameTransform(glm::vec3& position, glm::quat& rotation) {
		if (!m_rigidBody) return; // Only update if the body is active (Play Mode)

		btTransform trans;
		m_motionState->getWorldTransform(trans);

		// Read the physics state back to the game engine
		position = toGlmVec(trans.getOrigin());
		rotation = toGlmQuat(trans.getRotation());
	}

	void applyForce(const glm::vec3& force) {
		if (!m_rigidBody) return;

		m_rigidBody->applyCentralForce(toBtVec(force));
	}

	// Getter for the PhysicsSystem
	btRigidBody* getRigidBody() const { return m_rigidBody; }


	// utility methods
	static btVector3 toBtVec(const glm::vec3& v) {
		return btVector3(v.x, v.y, v.z);
	}

	// Convert GLM quaternion to Bullet quaternion
	static btQuaternion toBtQuat(const glm::quat& q) {
		return btQuaternion(q.x, q.y, q.z, q.w);
	}

	// Convert Bullet vector to GLM vector
	static glm::vec3 toGlmVec(const btVector3& v) {
		return glm::vec3(v.getX(), v.getY(), v.getZ());
	}

	// Convert Bullet quaternion to GLM quaternion
	static glm::quat toGlmQuat(const btQuaternion& q) {
		return glm::quat(q.w(), q.x(), q.y(), q.z());
	}

	static glm::quat EulerToQuat(const glm::vec3& eulerAnglesRadians) {

		// METHOD 1 (Standard GLM Conversion):
		// Assumes ZYX order (Yaw, Pitch, Roll) by default.
		// The components are often expected as (pitch, yaw, roll) depending on convention,
		// but glm::quat(vec3) generally handles (x_pitch, y_yaw, z_roll)
		// using an internal conversion based on ZYX matrix multiplication.
		// This is the simplest but least explicit.
		// return glm::quat(eulerAnglesRadians); 

		// METHOD 2 (Explicit Matrix Conversion - Recommended for Clarity):
		// This explicitly builds a rotation matrix from Euler angles (in ZYX order) 
		// and then converts the matrix to a quaternion.

		glm::vec3 degree = (glm::pi<float>() / 180.0f)*eulerAnglesRadians;
		glm::mat4 rotationMatrix = glm::eulerAngleYXZ(
			degree.y, // Yaw (Y-axis)
			degree.x, // Pitch (X-axis)
			degree.z  // Roll (Z-axis)
		);
		return glm::quat_cast(rotationMatrix);
	}

	static glm::vec3 QuatToEuler(const glm::quat& q) {
		// The eulerAngles function returns the Euler angles (pitch, yaw, roll) 
		// in RADIANS, generally following the YXZ convention (or similar) 
		// to minimize discontinuities.
		glm::vec3 degree = glm::eulerAngles(q) * (180 / glm::pi<float>());
		return degree;
	}
};