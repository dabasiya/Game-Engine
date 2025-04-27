#include <glad/glad.h>
#include <Renderer2D.h>
#include "Application.h"
#include <Event/Event.h>
#include <TimeStep.h>

#include <Scene/AnimationUtil.h>
#include <Scripts/ScriptManager.h>

#include <PerlinNoise.h>

// for testing later removed
Serializer Application::s_serializer;

bool Application::load_newscene = false;

bool Application::EditorMode = false;

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

SoundManager* Application::s_SoundManager;

Texture* Application::icons;


Application::Application() {

	m_Window = new Window("test", 1000, 1000);

	m_Window->SetEventCallBack(BIND_EVENT_FN(OnEvent));

	gladLoadGL();

	m_Shader = new Shader("res/shaders/color.vert", "res/shaders/color.frag");
        
	m_OrthographicCamera = new OrthographicCamera(Window::OrthographicSize, -1.0f, 1.0f);

	m_scene = new Scene();

	s_serializer = Serializer(m_scene);
	s_serializer.Deserialize("res/scene.vins");

	s_SoundManager = new SoundManager();

	scriptmanager::Init();

	m_fontrenderer = new FontRenderer(64, "res/fonts/ariali.ttf", 1);
	//m_fontrenderer = new FontRenderer(128, "res/fonts/times.ttf", 1);

	m_editorlayer = new EditorLayer();

	ui::fr = m_fontrenderer;
	Renderer2D::Init();
	Renderer2D::UseShader(m_Shader);

	PerlinNoise::initperm();

	icons = new Texture("res/icons.png");
	Renderer2D::SetTexture(icons, 15);
}



// on any event
void Application::OnEvent(Event& e) {
	if (e.is(Event_Type::Window_Resized)) {
		glViewport(0, 0, Window::Width, Window::Height);
		m_OrthographicCamera->Update();
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

			else if (ke->key == GLFW_KEY_U) {
				EditorLayer::camera_locked = !EditorLayer::camera_locked;
			}
		}
	}

	if (EditorMode)
		m_editorlayer->OnEvent(e);

	s_serializer.m_scene->OnEvent(e);

}


void Application::Run() {

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
	Renderer2D::SetTexture(m_fontrenderer->fonttexture, 1);
	m_scene->OnRuntimeStart();

	while (!m_Window->ShouldClose()) {

		float currenttime = glfwGetTime();
		float time = currenttime - lasttime;
		TimeStep ts(time);
		lasttime = currenttime;

		s_SoundManager->Update();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
                Renderer2D::UseShader(m_Shader);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		s_serializer.m_scene->Update(ts);

		if (EditorMode)
			m_editorlayer->Draw(ts.getmillisecond());

		if (load_newscene)
			loadsceneruntime_after_mainloop();

		Window::Update_MousePositions();
		m_Window->SwapBuffers();

	}

}

void Application::Release() {

	Renderer2D::Release();

	//delete m_scene;
	delete m_editorlayer;
	m_fontrenderer->Release();
	delete m_fontrenderer;
	delete m_Shader;
	delete m_Window;
	delete m_OrthographicCamera;
	delete s_SoundManager;

	m_Window->Destroy();
}

void Application::loadsceneruntime_after_mainloop() {
	// for check this called from editor or game runtime
	bool physics_exist = false;
	if (s_serializer.m_scene->physicsworld)
		physics_exist = true;

	if(physics_exist)
		s_serializer.m_scene->OnRuntimeStop();

	delete s_serializer.m_scene;

	s_serializer.m_scene = new Scene();

	s_serializer.Deserialize(s_serializer.filepath);

	if(physics_exist)
		s_serializer.m_scene->OnRuntimeStart();

	EditorLayer::selectedentity = { (entt::entity)0, nullptr };
	EditorLayer::selectedsubtexture = nullptr;

	load_newscene = false;
}

void Application::loadsceneruntime(const std::string& path) {
	s_serializer.filepath = path;
	load_newscene = true;
}
