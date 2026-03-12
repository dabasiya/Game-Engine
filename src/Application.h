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

enum RenderOutput {
	FINAL_OUTPUT = 0,
	SSAO,
	DEPTH
};

struct Application {

	static float gpudrawingtime;

	static bool captureshadowmap;

	static RenderOutput s_renderoutput;

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
	std::shared_ptr<Texture> bloomBuffer;
	std::shared_ptr<Texture> bloomverticalblurred;
	std::shared_ptr<Texture> bloomhorizontalblurred;
	std::shared_ptr<Texture> finalbloomblurred;

	std::shared_ptr<Texture> downsampledbloomBuffer;

	std::shared_ptr<FrameBuffer> downsampledbloomF;
	std::shared_ptr<FrameBuffer> bloomverticalF;
	std::shared_ptr<FrameBuffer> bloomhorizontalF;
	std::shared_ptr<FrameBuffer> finalbloomF;

	// for ssao
	std::shared_ptr<FrameBuffer> ssaoFBuffer;
	std::shared_ptr<Texture> ssaobuffer;

	// for blur ssao
	std::shared_ptr<FrameBuffer> ssaoblurFBuffer;
	std::shared_ptr<Texture> blurredssaobuffer;

	std::shared_ptr<FrameBuffer> ssaoblurFBuffer2;
	std::shared_ptr<Texture> blurredssaobuffer2;


	// for final image which is rendered to screen
	std::shared_ptr<FrameBuffer> finalFrameBuffer;
	std::shared_ptr<Texture> finalcolorTexture;

	// for store editormode enabled or not
	bool EditorMode;


	// icons for engine
	std::shared_ptr<Texture> icons;

	Serializer s_serializer;

	// for load scene at runtime
	void loadsceneruntime_after_mainloop();

	void loadsceneruntime(const std::string& path);

	void loadscenefrom_editor();
	bool load_newscene;

	void ReCreateGBuffer();
	void ReCreateSSAOBuffer();
	void ReCreateFinalFrameBuffer();

	// scene
	std::shared_ptr<Scene> m_scene;

	std::shared_ptr<Scene> m_ObjectSelectionScene;

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
