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

#include <Animation3D.h>
#include <Animator.h>

#include <FrameBuffer.h>

#include <btBulletDynamicsCommon.h>




enum CameraType {
	Orthographic = 0,
	Perspective = 1
};

enum LightType {
	DIRECTIONAL_LIGHT = 0,
	SPOT_LIGHT = 1,
	POINT_LIGHT = 2,
	NO_SHADOW_POINT_LIGHT = 3
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

// ...existing code...
struct TransformComponent {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f); // Euler degrees (for UI)
    glm::vec3 scale = glm::vec3(1.0f);

    // new: canonical rotation storage
    glm::quat rotationQuat = glm::quat(glm::vec3(0.0f));

    glm::mat4 worldtransform = glm::mat4(1.0f);

    // keep Euler and quat in sync
    void SetRotationEuler(const glm::vec3& degrees) {
        rotation = degrees;
        rotationQuat = glm::quat(glm::radians(rotation));
    }
    void SetRotationQuat(const glm::quat& q) {
        rotationQuat = q;
        //rotation = glm::degrees(glm::eulerAngles(rotationQuat));
    }

    // build local matrix using quaternion (T * R * S)
    glm::mat4 getmatrix() const {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 r = glm::mat4_cast(rotationQuat);
        glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
        return t * r * s;
    }


	// this is function is only for camera because camera use transformcomponent rotation for its view direction
	glm::vec3 direction() const {

		glm::vec3 radians = glm::radians(rotation);
		float pitch = radians.x;
		float yaw = radians.y;
		
		glm::vec3 direction;
		direction.x = -glm::sin(yaw) * glm::cos(pitch);
		direction.y = glm::sin(pitch);  // Negative because +pitch should look down
		direction.z = -glm::cos(yaw) * glm::cos(pitch);
		
		return glm::normalize(direction);
	}
};
// ...existing code...


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

struct UISpriteRendererComponent {

	SubTexture m_subtexture;

	bool type = 0; // by default its color type spriterenderercomponent


	float opacity = 1.0f;


	bool transparent = false;
	glm::vec4 color = glm::vec4(0.0f);

	UISpriteRendererComponent(const glm::vec4& a_color)
		: color(a_color), type(0)
	{

	}

	UISpriteRendererComponent(const SubTexture& a_subtexture, const glm::vec4& a_color, bool type)
		: m_subtexture(a_subtexture), color(a_color), type(type) {

	}

	UISpriteRendererComponent(const SubTexture& a_subtexture)
		: m_subtexture(a_subtexture), type(1)
	{
	}

	UISpriteRendererComponent()
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

struct UIFontRendererComponent {
	std::string text;

	float opacity;

	float pixelsize;

	UIFontRendererComponent() = default;

	UIFontRendererComponent(const std::string& a_text, float psize, float opacity = 1.0f)
		: text(a_text), opacity(opacity), pixelsize(psize)
	{
	}
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

	bool bloom = false;

	bool outline = false;
};


struct LightComponent {

	float shadoworthosize = 40.0f;
	float farplane = 100.0f;


	glm::vec4 color;

	LightType lighttype;

	// In Case of DirectionLight and SpotLight
	glm::vec3 direction;

	int depthcubesize = 256;

	// In Case of spotlight
	float angle;

	FrameBuffer m_FrameBuffer;
	std::shared_ptr<Texture> m_Texture;

	bool rerender = true;

	bool active = true;

	void EntityUpdate(const glm::vec3 &position, const glm::vec3 pos) {
		
		if (lighttype == LightType::DIRECTIONAL_LIGHT) {
			float w = (float)Window::Width / Window::Height;

			glm::mat4 proj = glm::ortho(-shadoworthosize * w, shadoworthosize * w, -shadoworthosize, shadoworthosize, 0.1f, farplane);

			glm::mat4 viewproj = proj * glm::lookAt(-direction * farplane, -direction * farplane + direction, glm::vec3(0.0f, 1.0f, 0.0f));

			glm::vec4 test = viewproj * glm::vec4(pos, 1.0f);

			if ((test.x >= -1.0f && test.x <= 1.0f) && (test.y >= -1.0f && test.y <= 1.0f)) {
				rerender = true;
			}
		}

		else if (lighttype == LightType::SPOT_LIGHT) {
			float w = (float)Window::Width / Window::Height;
			float h = 1.0f;

			glm::mat4 proj = glm::perspective(glm::radians((angle + 10.0f) * 2.0f), 1.0f, 1.0f, farplane);

			glm::mat4 view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));

			glm::mat4 viewproj = proj * view;

			glm::vec4 test = viewproj * glm::vec4(pos, 1.0f);

			test.x /= test.w;
			test.y /= test.w;

			if ((test.x >= -1.0f && test.x <= 1.0f) && (test.y >= -1.0f && test.y <= 1.0f)) {
				rerender = true;
			}
		}
		else {
			rerender = true;
		}
	}


	void Update() {
		
		if (m_Texture == nullptr) {
			rerender = true;
			if (lighttype == POINT_LIGHT)
				m_Texture = std::make_shared<Texture>(depthcubesize, depthcubesize, GL_DEPTH_COMPONENT32F);
			else
				m_Texture = std::make_shared<Texture>(Window::Width, Window::Height, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
			m_FrameBuffer.AddDepthAttachment(*m_Texture, GL_DEPTH_ATTACHMENT);
		}
		
		if ((Window::Width != m_Texture->width || Window::Height != m_Texture->height) && lighttype != POINT_LIGHT) {
			rerender = true;
			m_Texture.reset();
			m_Texture = std::make_shared<Texture>(Window::Width, Window::Height, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
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

	bool OnlyPositive = false;

	PhysicsShapeType ShapeType = PhysicsShapeType::Box;

	// Box properties
	glm::vec3 BoxHalfExtents = glm::vec3(0.5f, 0.5f, 0.5f);

	bool Rotations[3] = { true, true, true };

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

		btVector3 brotations(Rotations[0] ? 1.0f : 0.0f, Rotations[1] ? 1.0f : 0.0f , Rotations[2] ? 1.0f : 0.0f);

		m_rigidBody->setAngularFactor(brotations);

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

		if (OnlyPositive) {
			if (ShapeType == PhysicsShapeType::Box)
				position.y -= BoxHalfExtents.y;
			else if (ShapeType == PhysicsShapeType::Sphere)
				position.y -= SphereRadius;
		}
	}

	void applyForce(const glm::vec3& force) {
		if (!m_rigidBody) {
			std::cout << "body is not created!" << std::endl;
			return;
		}


		m_rigidBody->activate(true);
		m_rigidBody->applyCentralForce(toBtVec(force));
	}

	void applyImpulseForce(const glm::vec3& force) {
		if (!m_rigidBody) {
			std::cout << "body is not created!" << std::endl;
			return;
		}
		m_rigidBody->activate(true);
		m_rigidBody->applyCentralImpulse(toBtVec(force));
	}

	glm::vec3 GetVelocity() {
		if (!m_rigidBody)
			return glm::vec3(0.0f);

		btVector3 vel = m_rigidBody->getLinearVelocity();
		return toGlmVec(vel);
	}

	void SetVelocity(glm::vec3 velocity) {
		if (!m_rigidBody)
			return;
		
		m_rigidBody->activate(true);
		m_rigidBody->setLinearVelocity(toBtVec(velocity));
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

		

		glm::vec3 degree = (glm::pi<float>() / 180.0f)*eulerAnglesRadians;
		glm::mat4 rotationMatrix = glm::eulerAngleYXZ(
			degree.y, // Yaw (Y-axis)
			degree.x, // Pitch (X-axis)
			degree.z  // Roll (Z-axis)
		);
		return glm::quat_cast(rotationMatrix);
	}

	static glm::vec3 QuatToEuler(const glm::quat& q) {

		// 1. Get the raw Euler angles in RADIANS (pitch, yaw, roll)
		glm::vec3 eulerRadians = glm::eulerAngles(q);

		// 2. Create the new vector, swapping Y (Yaw) and Z (Roll) components.
		glm::vec3 swappedEulerRadians = glm::vec3(
			eulerRadians.x, // X (Pitch) remains X
			eulerRadians.y, // Y is now the original Z (Roll)
			eulerRadians.z  // Z is now the original Y (Yaw)
		);

		// 3. Convert the resulting vector to DEGREES for output
		glm::vec3 eulerDegrees = swappedEulerRadians * (180.0f / glm::pi<float>());

		return eulerDegrees;
	}
};


struct Animation3DComponent {
	
	std::unordered_map <std::string, std::shared_ptr<Animation3D>> mAnimation3DMap;
	std::unordered_map <std::string, std::shared_ptr<Animator>> mAnimatorMap;
	std::vector<std::string> mAnimationNames;

	std::vector<bool> activeanimatioins;

	unsigned int activeanimationindex = 0;

	bool isFirsttime = true;

	unsigned int UBO;


	void Reload(const std::string& path, Model* model) {
		mAnimation3DMap.clear();
		mAnimatorMap.clear();
		mAnimationNames.clear();
		activeanimatioins.clear();

		activeanimationindex = 0;

		mAnimationNames.push_back("No Animation");
		activeanimatioins.push_back(true);

		unsigned int index = 0;
		while (true) {
			
			std::string animationname = Animation3D::GetAnimationName(path, index);

			if (animationname == "")
				break;

			mAnimation3DMap[animationname] = std::make_shared<Animation3D>(path, model, index);
			mAnimatorMap[animationname] = std::make_shared<Animator>(mAnimation3DMap[animationname].get());
			mAnimationNames.push_back(animationname);
			activeanimatioins.push_back(false);


			index++;
		}


		// create UBO

		glCreateBuffers(1, &UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferData(GL_UNIFORM_BUFFER, 300 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
	}

	void ResetAnimation(const std::string& name) {
		if (mAnimatorMap.find(name) == mAnimatorMap.end())
			return;

		mAnimatorMap[name]->Reset();
	}

	void UpdateAnimation(float ts) {
		
		if (mAnimatorMap.find(mAnimationNames[activeanimationindex]) == mAnimatorMap.end())
			return;

		mAnimatorMap[mAnimationNames[activeanimationindex]]->UpdateAnimation(ts);
	}

	inline std::vector<glm::mat4> GetFinalBoneMetrices() {
		std::string name = mAnimationNames[activeanimationindex];
		if (mAnimatorMap.find(name) == mAnimatorMap.end())
			return { glm::mat4(1.0f) };

		return mAnimatorMap[name]->GetFinalBoneMetrices();
	}

	void setActive(unsigned int index) {
		unsigned int len = mAnimationNames.size();

		if (index >= len)
			return;

		for (unsigned int i = 0; i < len; i++)
			activeanimatioins[i] = false;

		activeanimatioins[index] = true;
		activeanimationindex = index;
	}


	void SetBoneMetrices(std::shared_ptr<Shader> shader) {

		if (isFirsttime) {
			glBindBuffer(GL_UNIFORM_BUFFER, UBO);
			int index = shader->GetUniformBlockIndex("BoneMetrices");
			glUniformBlockBinding(shader->ID, index, 0);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
			isFirsttime = false;
		}

		unsigned int index = 1;
		unsigned  int len = mAnimationNames.size();
		for (unsigned int i = 0; i < len; i++) {
			if (activeanimatioins[i]) {
				index = i;
				break;
			}
		}

		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		auto bones = GetFinalBoneMetrices();
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * bones.size(), bones.data());
	}
};




struct CachedComponents {

	// for track record of available component

	bool havecameracomponent;
	bool havespriterenderercomponent;
	bool havefontrenderercomponent;
	bool havescriptcomponent;
	bool haveanimationgroupcomponent;
	bool haverigidbody2dcomponent;
	bool haveboxcollider2dcomponent;
	bool haveparticlegeneratorcomponent;
	bool havemodel3dcomponent;
	bool havelightcomponent;
	bool havephysicscomponent;
	bool haveanimation3dcomponent;

};