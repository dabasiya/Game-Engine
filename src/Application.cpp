#include <glad/glad.h>
#include <Renderer2D.h>
#include "Application.h"
#include <Event/Event.h>
#include <TimeStep.h>
#include <Scene/AnimationUtil.h>
#include <Scripts/ScriptManager.h>
#include <Scene/Components.h>
#include <ShaderManager.h>
#include <FontManager.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>


#include <Instrumentor.h>

RenderOutput Application::s_renderoutput = FINAL_OUTPUT;



float ParticleSystem::yrotation = 0.0f;

bool Application::captureshadowmap = true;

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)


float Application::gpudrawingtime = 10.0f;

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
	ShaderManager::Add("ssao", "res/shaders/ssao.vs", "res/shaders/ssao.fs");
	ShaderManager::Add("linearblur", "res/shaders/linearblur.vs", "res/shaders/linearblur.fs");
	ShaderManager::Add("2dimage", "res/shaders/2dimage.vert", "res/shaders/2dimage.frag");
	ShaderManager::Add("computeblur", "res/shaders/computeblur.cshader");
	ShaderManager::Add("downsample", "res/shaders/2dimage.vert", "res/shaders/upsample.frag");
	ShaderManager::Add("rgbablur", "res/shaders/2dimage.vert", "res/shaders/rgbablur.frag");
	ShaderManager::Add("upsample", "res/shaders/2dimage.vert", "res/shaders/upsample.frag");
	ShaderManager::Add("finalbloom", "res/shaders/2dimage.vert", "res/shaders/finalbloom.frag");
	ShaderManager::Add("outline", "res/shaders/shadowmap.vert", "res/shaders/outline.frag");

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

	// allocate scene for object selection scene
	m_ObjectSelectionScene = std::make_shared<Scene>();

	s_serializer = Serializer(m_ObjectSelectionScene);

	s_serializer.Deserialize("res/ObjectSelection.scene");

	// create serializer for save and load scene from disk
	s_serializer.m_scene = m_scene;
	//s_serializer.Deserialize("res/model/model.scene");
	s_serializer.Deserialize("res/example.scene");
	//s_serializer.Deserialize("res/level.scene");
	//s_serializer.Deserialize("res/monopoly.scene");

	// Create GBuffer for Deferred Renderer
	gBuffer = std::make_shared<FrameBuffer>();
	downsampledbloomF = std::make_shared<FrameBuffer>();
	bloomhorizontalF = std::make_shared<FrameBuffer>();
	bloomverticalF = std::make_shared<FrameBuffer>();
	finalbloomF = std::make_shared<FrameBuffer>();
	ReCreateGBuffer();

	ssaoFBuffer = std::make_shared<FrameBuffer>();

	ssaoblurFBuffer = std::make_shared<FrameBuffer>();

	ssaoblurFBuffer2 = std::make_shared<FrameBuffer>();

	ReCreateSSAOBuffer();

	finalFrameBuffer = std::make_shared<FrameBuffer>();
	ReCreateFinalFrameBuffer();
	 
	// Initalize scripts object 
	scriptmanager::Init();

	

	// Create EditorLayer
	m_editorlayer = std::make_shared<EditorLayer>();

	// Initalize 2D Renderer
	Renderer2D::Init();
	Renderer2D::UseShader(ShaderManager::Get("2d"));

	// create fontrenderer and assign that fonts to ui for rendering fonts of ui
	//m_fontrenderer = std::make_shared<FontRenderer>(16, "res/fonts/times.ttf", 1);
	ui::fr = FontManager::GetFont(16, "res/fonts/times.ttf");

	icons = std::make_shared<Texture>("res/icons.png");
	Renderer2D::SetTexture(icons, 15);
}


// GBuffer Creation function which will create gbuffer at first or when window size changed
void Application::ReCreateGBuffer() {

	PositionBuffer.reset();
	NormalBuffer.reset();
	DepthBuffer.reset();
	AlbedoBuffer.reset();
	downsampledbloomBuffer.reset();
	bloomverticalblurred.reset();
	bloomhorizontalblurred.reset();
	finalbloomblurred.reset();
	bloomBuffer.reset();

	gBuffer->Clear();
	downsampledbloomF->Clear();
	bloomhorizontalF->Clear();
	bloomverticalF->Clear();
	finalbloomF->Clear();


	bloomBuffer = std::make_shared<Texture>(Window::Width, Window::Height, GL_RGBA8, GL_RGBA, 0);
	PositionBuffer = std::make_shared<Texture>(Window::Width, Window::Height, GL_RGB16F, GL_RGB, 0);
	NormalBuffer = std::make_shared<Texture>(Window::Width, Window::Height, GL_RGB8, GL_RGB, 0);
	DepthBuffer = std::make_shared<Texture>(Window::Width, Window::Height, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
	AlbedoBuffer = std::make_shared<Texture>(Window::Width, Window::Height, GL_RGBA8, GL_RGBA, 0);
	downsampledbloomBuffer = std::make_shared<Texture>(Window::Width / 2, Window::Height / 2, GL_RGBA8, GL_RGBA, 0);
	bloomhorizontalblurred = std::make_shared<Texture>(Window::Width / 2, Window::Height / 2, GL_RGBA8, GL_RGBA, 0);
	bloomverticalblurred = std::make_shared<Texture>(Window::Width / 2, Window::Height / 2, GL_RGBA8, GL_RGBA, 0);
	finalbloomblurred = std::make_shared<Texture>(Window::Width, Window::Height, GL_RGBA8, GL_RGBA, 0);

	// gbuffer
	gBuffer->AddColorAttachment(0, *PositionBuffer);
	gBuffer->AddColorAttachment(1, *NormalBuffer);
	gBuffer->AddColorAttachment(2, *AlbedoBuffer);
	gBuffer->AddColorAttachment(3, *bloomBuffer);
	gBuffer->AddDepthAttachment(*DepthBuffer, GL_DEPTH_STENCIL_ATTACHMENT);

	downsampledbloomF->AddColorAttachment(0, *downsampledbloomBuffer);
	bloomhorizontalF->AddColorAttachment(0, *bloomhorizontalblurred);
	bloomverticalF->AddColorAttachment(0, *bloomverticalblurred);
	finalbloomF->AddColorAttachment(0, *finalbloomblurred);
}


void Application::ReCreateSSAOBuffer() {
	ssaobuffer.reset();
	blurredssaobuffer.reset();
	blurredssaobuffer2.reset();

	ssaoFBuffer->Clear();
	ssaoblurFBuffer->Clear();
	ssaoblurFBuffer2->Clear();

	ssaobuffer = std::make_shared<Texture>(Window::Width / 4, Window::Height / 4, GL_R8, GL_R, 0);
	blurredssaobuffer = std::make_shared<Texture>(Window::Width / 4, Window::Height / 4, GL_R8, GL_R, 0);
	blurredssaobuffer2 = std::make_shared<Texture>(Window::Width / 4, Window::Height / 4, GL_R8, GL_R, 0);

	ssaoFBuffer->AddColorAttachment(0, *ssaobuffer);
	ssaoblurFBuffer->AddColorAttachment(0, *blurredssaobuffer);
	ssaoblurFBuffer2->AddColorAttachment(0, *blurredssaobuffer2);
}

void Application::ReCreateFinalFrameBuffer() {
	finalcolorTexture.reset();
	//bloomBuffer.reset();

	finalFrameBuffer->Clear();

	finalcolorTexture = std::make_shared<Texture>(Window::Width, Window::Height, GL_RGBA8, GL_RGBA, 0);
	// = std::make_shared<Texture>(Window::Width, Window::Height, GL_RGBA8, GL_RGBA, 0);

	finalFrameBuffer->AddColorAttachment(0, *finalcolorTexture);
	finalFrameBuffer->AddDepthAttachment(*DepthBuffer, GL_DEPTH_STENCIL_ATTACHMENT);
}


// on any event
void Application::OnEvent(Event& e) {

	HZ_PROFILE_SCOPE("Application Event");

	if (e.is(Event_Type::Window_Resized)) {
		glViewport(0, 0, Window::Width, Window::Height);
		return;
	}

	if (e.is(Event_Type::Key_Pressed)) {
		KeyPressedEvent* ke = (KeyPressedEvent*)&e;

		if (ke->mod & GLFW_MOD_CONTROL) {
			if (ke->key == GLFW_KEY_E) {
				EditorMode = !EditorMode;
				if (EditorMode) {
					glfwSetInputMode(Window::ID, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					s_serializer.m_scene->OnRuntimeStop();
					m_editorlayer->selectedentity->m_scene = nullptr;
					return;
				}
				else {
					glfwSetInputMode(Window::ID, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					s_serializer.m_scene->OnRuntimeStart();
					return;
				}
			}

			else if (ke->key == GLFW_KEY_U && EditorMode == true) {
				EditorLayer::camera_locked = !EditorLayer::camera_locked;
				return;
			}
		}
	}

	if (EditorMode) {
		m_editorlayer->OnEvent(e);

	}

	s_serializer.m_scene->OnEvent(e);

}


void Application::Run() {

	ui::Init();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	//Renderer2D::SetTexture(m_fontrenderer->fonttexture, 1);
	m_scene->OnRuntimeStart();



	// for imgui
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(m_Window->ID, true);
	ImGui_ImplOpenGL3_Init(shader_version);

	// Application Loop
	while (!m_Window->ShouldClose()) {

		glfwPollEvents();

		glViewport(0, 0, Window::Width, Window::Height);

		HZ_PROFILE_SCOPE("frame");

		// If Window Size Changed then ReCreateGBuffer
		if (PositionBuffer->width != Window::Width || PositionBuffer->height != Window::Height) {
			ReCreateGBuffer();
			ReCreateSSAOBuffer();
			ReCreateFinalFrameBuffer();
		}

		glEnable(GL_DEPTH_TEST);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float currenttime = glfwGetTime();
		float time = currenttime - lasttime;
		TimeStep ts(time);
		lasttime = currenttime;

		auto smanager = SoundManager::GetInstance();
		smanager->Update();

		if (EditorMode && EditorLayer::showobjectselectionscene) {
			s_serializer.m_scene = m_ObjectSelectionScene;
		}
		else {
			s_serializer.m_scene = m_scene;
		}


		glBindFramebuffer(GL_FRAMEBUFFER, 0);	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		{
			HZ_PROFILE_SCOPE("Scene render");
			s_serializer.m_scene->Update(ts);
		}

		glDepthFunc(GL_LEQUAL);

		// draw and handle gizmos
		if (EditorMode)
			m_editorlayer->imguizmos();

		// draw editorlayer if enabled
		//std::cout << EditorMode << std::endl;
		if (EditorMode) {
			HZ_PROFILE_SCOPE("EditorLayer");
			m_editorlayer->Draw(ts);
		}
		
		// load newscene if new entered in input
		if (load_newscene)
			loadsceneruntime_after_mainloop();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		Window::Update_MousePositions();

		{
			HZ_PROFILE_SCOPE("swap buffers");
			m_Window->SwapBuffers();
		}
	}



}

void Application::Release() {

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	Renderer2D::Release();

	//m_fontrenderer->Release();

	m_Window->Destroy();
}

void Application::loadsceneruntime_after_mainloop() {
	// for check this called from editor or game runtime
	s_serializer.m_scene->OnRuntimeStop();

	s_serializer.m_scene.reset();

	m_scene = std::make_shared<Scene>();

	s_serializer.m_scene = m_scene;

	s_serializer.Deserialize(s_serializer.filepath);

	if (!EditorMode)
		s_serializer.m_scene->OnRuntimeStart();


	// reset selectedentity in editorlayer
	m_editorlayer->selectedentity->id = (entt::entity)0;
	m_editorlayer->selectedentity->m_scene = nullptr;
	m_editorlayer->selectedsubtexture = nullptr;

	Application::captureshadowmap = true;

	load_newscene = false;
}

void Application::loadsceneruntime(const std::string& path) {
	std::cout << path << std::endl;
	s_serializer.filepath = path;
	load_newscene = true;
}
