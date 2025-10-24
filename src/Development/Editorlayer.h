#pragma once


#include <UI/UI.h>
#include <Scene/Entity.h>
#include <Event/Event.h>
#include <UI/UIElementData.h>
#include <Animation.h>
#include "Console.h"
#include <ParticleSystem.h>
#include <FrameBuffer.h>

#include <Scene/CopiedEntity.h>

#include <Development/EntityRegistrar.h>

struct CameraTransform {
	glm::vec3 position;
	glm::vec3 rotation;
};


struct EntitySceneEvent {
	bool MouseDown = false;
	unsigned char Axis = 0;
};

enum EntityGizmos {
	X_AXIS = 0,
	Y_AXIS = 1
};

enum EntityEditingMode {

	TRANSLATE = 0,
	SCALE = 1,
	ROTATE = 2
};

enum GizmoMode {
	ObjectMode = 0,
	ColliderMode = 1
};

enum EditorCameraType {
	TWO_AXIS_MOVE_CAMERA,
	THREE_AXIS_MOVE_CAMERA
};

struct EditorLayer {

	// editorcamera type
	static EditorCameraType s_EditorCameraType;
	static float EditorCameraSpeed;
	static CameraTransform s_CameraTransform;

	// for color input
	std::vector<std::string> colorvaluesstring = { "R", "G", "B", "A" };

	// for vector 3 input
	std::vector<std::string> positionvaluesstring = {
		  "X",
		  "Y",
		  "Z"
	};

	static CopiedEntity s_CopiedEntity;

	static bool was_mousepressed;

	static bool mouse_on_window;

	static SubTexture particlegeneratorsubtexture;

	// selected entity
	// for show properties in propertieswindow
	static std::shared_ptr<Entity> selectedentity;

	static SubTexture* selectedsubtexture;

	static bool camera_locked;

	static GizmoMode s_GizmoMode;

	static unsigned int s_EntityEditingMode;

	static bool showcolliders;


	Console test_console;

	EntityRegistrar m_EntityRegistrar;


	// framebuffer to store entity color so it can be used for mouse picking
	FrameBuffer m_entitybuffer;

	std::shared_ptr<Shader> m_entitycolorshader;

	std::unique_ptr<Texture> m_colorbuffer;
	std::unique_ptr<Texture> m_depthbuffer;

	EntitySceneEvent m_EntitySceneEvent;


	std::unordered_map<std::string, SubTexture> EditorLayerIcons;

	// for show windows some part

	bool position = false;
	bool rotation = false;
	bool scale = false;
	bool cameraprop = false;

	bool colorbegin = true;

	std::vector<std::string> rigidbodytypes = {
	  "Static",
	  "Dynamic"
	};

	// animation struct for create animation by animation creator window
	Animation m_animation;
	bool animationplaying = false;

	unsigned int rigidbodytypeindex = 0;

	unsigned int spriterenderertypeindex = 0;

	// animation creator window stuff
	std::string animationfilepath;


	windowdata windowmanagerdata = windowdata(0, 0, 400, { 0.05f, 0.32f, 0.03f, 1.0f }, { 0.1f, 0.56f, 0.07f, 1.0f });

	windowdata scenehierarchywindowdata = windowdata(0, 0, 400, { 0.32f, 0.05f, 0.03f, 1.0f }, { 0.56f, .1f, .07f, 1.0f });

	windowdata propertieswindowdata = windowdata(0, 0, 400, { 0.05f, 0.03f, 0.32f, 1.0f }, { .1f, .07f, .56f, 1.0f });

	windowdata subtexturewindowdata = windowdata(0, 0, 400, { 0.62f, 0.59f, 0.0f, 1.0f }, { 0.75f, 0.72f, 0.0f, 1.0f });

	windowdata animationcreatorwindowdata = windowdata(0, 0, 400, { .1f, .1f, .1f, 1.0f }, { .2f, .2f, .2f, 1.0f });

	windowdata texturewindowdata = windowdata(0, 0, 800, { 0.0f, .43f, .57f, 1.0f }, { .35f, .48f, .55f, 1.0f });

	windowdata entityregistrarwindowdata = windowdata(0, 0, 400, { 0.87f, .70f, 0.23, 1.0f }, { 0.88f, 0.75f, 0.38f, 1.0f });

	windowdata skyboxwindowdata = windowdata(0, 0, 600, { 0.6f, 0.0f, 0.24f, 1.0f }, { 1.0f, 0.0f, 0.4f, 1.0f });

	EditorLayer();
	~EditorLayer();

	bool OnEvent(Event& e);

	void Draw(float ts);


	bool MoveEntity();
	void SelectEntity();

	void CopyEntity();
	void PasteEntity();

	void imguizmos();

	void RenderEntityHierarchyTree(std::shared_ptr<Entity> entity, unsigned int offset);
};
