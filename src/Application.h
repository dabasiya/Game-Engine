#pragma once


#include <Scene/Entity.h>
#include <Scene/Components.h>
#include <OrthographicCamera.h>
#include <Shader.h>
#include <Scene/Scene.h>
#include <Event/Event.h>
#include <FontRenderer.h>
#include <Development/Editorlayer.h>
#include <Scene/Serializer.h>

#include <Sound/SoundManager.h>


struct Application {

	// mainwindow for display
	Window* m_Window;

	// shader
	Shader* m_Shader;

	// orthographic camera
	OrthographicCamera* m_OrthographicCamera;

	// for store editormode enabled or not
	static bool EditorMode;

	Entity e2;

	static Texture* icons;

	static Serializer s_serializer;

	// soundmanager

	static SoundManager* s_SoundManager;

	// for load scene at runtime
	void loadsceneruntime_after_mainloop();

	static void loadsceneruntime(const std::string& path);

	static void loadscenefrom_editor();
	static bool load_newscene;

	// scene
	Scene* m_scene;

	// fontrender
	FontRenderer* m_fontrenderer;


	// testing
	Texture* test_texture;

	// editorlayer
	EditorLayer* m_editorlayer;

	float lasttime = 0.0f;


	Application();

	void OnEvent(Event& e);

	void Run();

	void Release();
};
