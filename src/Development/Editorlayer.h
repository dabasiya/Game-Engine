#pragma once


#include <UI/UI.h>
#include <Scene/Entity.h>
#include <Event/Event.h>
#include <UI/UIElementData.h>
#include <Animation.h>
#include "Console.h"
#include <ParticleSystem.h>
#include <FrameBuffer.h>


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


struct EditorLayer {


	// for color input
	std::vector<std::string> colorvaluesstring = { "R", "G", "B", "A" };

	// for vector 3 input
	std::vector<std::string> positionvaluesstring = {
		  "X",
		  "Y",
		  "Z"
	};


	static bool was_mousepressed;

	static bool record_chain_collider_vertices;

	static bool mouse_on_window;

	static SubTexture particlegeneratorsubtexture;

	// selected entity
	// for show properties in propertieswindow
	static Entity selectedentity;

	static SubTexture* selectedsubtexture;

	static bool camera_locked;

	static unsigned int s_EntityEditingMode;

	static bool showcolliders;

	Console test_console;


  // framebuffer to store entity color so it can be used for mouse picking
  FrameBuffer m_entitybuffer;

   Shader* m_entitycolorshader;

  Texture* m_colorbuffer;
  Texture* m_depthbuffer;

	EntitySceneEvent m_EntitySceneEvent;

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
	
  EditorLayer();

	bool OnEvent(Event& e);

	void Draw(float ts);


	bool MoveEntity();
	void SelectEntity();
};
