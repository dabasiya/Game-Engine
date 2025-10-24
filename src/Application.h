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
#include <SkyBox.h>


struct Application {

	// mainwindow for display
	std::shared_ptr<Window> m_Window;

	// skybox
	SkyBox* skybox;

	// framebuffer for g buffer
	std::shared_ptr<FrameBuffer> gBuffer;
	std::shared_ptr<Texture> PositionBuffer;
	std::shared_ptr<Texture> NormalBuffer;
	std::shared_ptr<Texture> DepthBuffer;
	std::shared_ptr<Texture> AlbedoBuffer;


	// for store editormode enabled or not
	bool EditorMode;


	// icons for engine
	std::shared_ptr<Texture> icons;

	Serializer s_serializer;

	// soundmanager

	SoundManager* s_SoundManager;

	// for load scene at runtime
	void loadsceneruntime_after_mainloop();

	void loadsceneruntime(const std::string& path);

	void loadscenefrom_editor();
	bool load_newscene;

	void ReCreateGBuffer();

	// scene
	std::shared_ptr<Scene> m_scene;

	// fontrender
	std::shared_ptr<FontRenderer> m_fontrenderer;



	// editorlayer
	std::shared_ptr<EditorLayer> m_editorlayer;

	float lasttime = 0.0f;

	void OnEvent(Event& e);

	void Run();

	void Release();

	static Application& GetInstance();

	const char* shader_version = "#version 330 core";


private:

	Application();
};
