#include <glad/glad.h>
#include <Renderer2D.h>
#include "Application.h"
#include <Event/Event.h>
#include <TimeStep.h>
#include <Scene/AnimationUtil.h>
#include <Scripts/ScriptManager.h>
#include <Scene/Components.h>
#include <ShaderManager.h>


#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>


#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)


// for getting instance
Application& Application::GetInstance() {
	static Application app;
	return app;
}

Application::Application() {

	EditorMode = false;

	m_Window = std::make_shared<Window>(Window("test", 1000, 1000));

	m_Window->SetEventCallBack(BIND_EVENT_FN(OnEvent));

	gladLoadGL();

	// allocate shaders
	ShaderManager::Add("2d", "res/shaders/color.vert", "res/shaders/color.frag");
	ShaderManager::Add("2dparticle", "res/shaders/particle.vert", "res/shaders/particle.frag");
	ShaderManager::Add("3d", "res/shaders/3d.vs", "res/shaders/3d.fs");
	ShaderManager::Add("skybox", "res/shaders/skybox.vert", "res/shaders/skybox.frag");
	ShaderManager::Add("shadowmap", "res/shaders/shadowmap.vert", "res/shaders/shadowmap.frag");
	ShaderManager::Add("cubeshadowmap", "res/shaders/cubeshadowmap.vert", "res/shaders/cubeshadowmap.geometry", "res/shaders/cubeshadowmap.frag");
	ShaderManager::Add("3dline", "res/shaders/3dline.vs", "res/shaders/3dline.frag");
	ShaderManager::Add("gbuffer", "res/shaders/gbuffer.vs", "res/shaders/gbuffer.fs");
	ShaderManager::Add("3dfromgbuffer", "res/shaders/3dfromgbuffer.vs", "res/shaders/3dfromgbuffer.fs");

	// allocate skybox
	skybox = new SkyBox();


	skybox->AddTexture(0, "skybox/right.jpg");
	skybox->AddTexture(1, "skybox/left.jpg");
	skybox->AddTexture(2, "skybox/top.jpg");
	skybox->AddTexture(3, "skybox/bottom.jpg");
	skybox->AddTexture(4, "skybox/front.jpg");
	skybox->AddTexture(5, "skybox/back.jpg");


	//allocate default scene memory
	m_scene = std::make_shared<Scene>();

	// create serializer for save and load scene from disk
	s_serializer = Serializer(m_scene);
	//s_serializer.Deserialize("res/model/model.scene");
	s_serializer.Deserialize("res/example.scene");
	//s_serializer.Deserialize("res/level.scene");
	//s_serializer.Deserialize("res/monopoly.scene");

	// Create GBuffer for Deferred Renderer
	gBuffer = std::make_shared<FrameBuffer>();
	ReCreateGBuffer();

	// For Sound
	s_SoundManager = new SoundManager();
	 
	// Initalize scripts object 
	scriptmanager::Init();

	// create fontrenderer and assign that fonts to ui for rendering fonts of ui
	m_fontrenderer = std::make_shared<FontRenderer>(32, "res/fonts/ariali.ttf", 1);
	ui::fr = m_fontrenderer;

	// Create EditorLayer
	m_editorlayer = std::make_shared<EditorLayer>();

	// Initalize 2D Renderer
	Renderer2D::Init();
	Renderer2D::UseShader(ShaderManager::Get("2d"));

	icons = std::make_shared<Texture>("res/icons.png");
	Renderer2D::SetTexture(icons, 15);
}


// GBuffer Creation function which will create gbuffer at first or when window size changed
void Application::ReCreateGBuffer() {

	PositionBuffer.reset();
	NormalBuffer.reset();
	DepthBuffer.reset();
	AlbedoBuffer.reset();

	PositionBuffer = std::make_shared<Texture>(Window::Width, Window::Height, GL_RGB16F, GL_RGB, 0);
	NormalBuffer = std::make_shared<Texture>(Window::Width, Window::Height, GL_RGB16F, GL_RGB, 0);
	DepthBuffer = std::make_shared<Texture>(Window::Width, Window::Height, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
	AlbedoBuffer = std::make_shared<Texture>(Window::Width, Window::Height, GL_RGBA8, GL_RGBA, 0);
	// gbuffer
	gBuffer->AddColorAttachment(0, *PositionBuffer);
	gBuffer->AddColorAttachment(1, *NormalBuffer);
	gBuffer->AddColorAttachment(2, *AlbedoBuffer);
	gBuffer->AddDepthAttachment(*DepthBuffer, GL_DEPTH_STENCIL_ATTACHMENT);
}



// on any event
void Application::OnEvent(Event& e) {
	if (e.is(Event_Type::Window_Resized)) {
		glViewport(0, 0, Window::Width, Window::Height);
	}

	if (e.is(Event_Type::Key_Pressed)) {
		KeyPressedEvent* ke = (KeyPressedEvent*)&e;

		if (ke->mod & GLFW_MOD_CONTROL) {
			if (ke->key == GLFW_KEY_E) {
				EditorMode = !EditorMode;
				if (EditorMode) {
					s_serializer.m_scene->OnRuntimeStop();
				}
				else {
					s_serializer.m_scene->OnRuntimeStart();
				}
			}

			else if (ke->key == GLFW_KEY_U && EditorMode == true) {
				EditorLayer::camera_locked = !EditorLayer::camera_locked;
			}
		}
	}

	if (EditorMode) {
		m_editorlayer->OnEvent(e);

	}
	s_serializer.m_scene->OnEvent(e);

}


void Application::Run() {

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
	glDisable(GL_CULL_FACE);
	Renderer2D::SetTexture(m_fontrenderer->fonttexture, 1);
	m_scene->OnRuntimeStart();



	// for imgui
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(m_Window->ID, true);
	ImGui_ImplOpenGL3_Init(shader_version);


	// Application Loop
	while (!m_Window->ShouldClose()) {


		// If Window Size Changed then ReCreateGBuffer
		if (PositionBuffer->width != Window::Width || PositionBuffer->height != Window::Height) {
			ReCreateGBuffer();
		}

		glEnable(GL_DEPTH_TEST);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float currenttime = glfwGetTime();
		float time = currenttime - lasttime;
		TimeStep ts(time);
		lasttime = currenttime;

		s_SoundManager->Update();


		glBindFramebuffer(GL_FRAMEBUFFER, 0);	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		s_serializer.m_scene->Update(ts);

		// draw and handle gizmos
		if (EditorMode)
			m_editorlayer->imguizmos();

		// draw editorlayer if enabled
		//std::cout << EditorMode << std::endl;
		if (EditorMode)
			m_editorlayer->Draw(ts.getmillisecond());
		
		// load newscene if new entered in input
		if (load_newscene)
			loadsceneruntime_after_mainloop();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		Window::Update_MousePositions();
		m_Window->SwapBuffers();

	}



}

void Application::Release() {

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	Renderer2D::Release();

	m_fontrenderer->Release();
	delete s_SoundManager;

	m_Window->Destroy();
}

void Application::loadsceneruntime_after_mainloop() {
	// for check this called from editor or game runtime
	bool physics_exist = false;
	if (s_serializer.m_scene->physicsworld)
		physics_exist = true;

	if (physics_exist)
		s_serializer.m_scene->OnRuntimeStop();

	s_serializer.m_scene.reset();

	s_serializer.m_scene = std::make_shared<Scene>();

	s_serializer.Deserialize(s_serializer.filepath);

	if (physics_exist && !EditorMode)
		s_serializer.m_scene->OnRuntimeStart();


	// reset selectedentity in editorlayer
	m_editorlayer->selectedentity->id = (entt::entity)0;
	m_editorlayer->selectedentity->m_scene = nullptr;
	m_editorlayer->selectedsubtexture = nullptr;

	load_newscene = false;
}

void Application::loadsceneruntime(const std::string& path) {
	s_serializer.filepath = path;
	load_newscene = true;
}
