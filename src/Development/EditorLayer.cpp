#include "Editorlayer.h"
#include <Scene/Components.h>
#include <Renderer2D.h>
#include <Application.h>
#include <Scene/AnimationUtil.h>
#include <WindowsInput.h>
#include <ModelManager.h>
#include <glm/gtc/type_ptr.hpp>
#include <gladr.h>

#include <imgui.h>
#include <ImGuizmo.h>

#include <Math/Math.h>
#include <algorithm>
#include <ShaderManager.h>

#include <Instrumentor.h>

std::vector<const char*> renderoutputstr = {
	"Final Output",
	"SSAO",
	"Depth"
};


EditorCameraType EditorLayer::s_EditorCameraType = TWO_AXIS_MOVE_CAMERA;
float EditorLayer::EditorCameraSpeed = 1.0f;
std::shared_ptr<Entity> EditorLayer::selectedentity = std::make_shared<Entity>();

#define selectedscene Application::GetInstance().s_serializer.m_scene

SubTexture* EditorLayer::selectedsubtexture = nullptr;
bool EditorLayer::was_mousepressed = false;
bool EditorLayer::mouse_on_window = false;
GizmoMode EditorLayer::s_GizmoMode = ObjectMode;
unsigned int EditorLayer::s_EntityEditingMode = 0;
bool EditorLayer::camera_locked = false;
bool EditorLayer::showcolliders = false;

bool EditorLayer::showobjectselectionscene = false;


SubTexture EditorLayer::particlegeneratorsubtexture;
CopiedEntity EditorLayer::s_CopiedEntity;
CameraTransform EditorLayer::s_CameraTransform;



std::shared_ptr<Entity> deleteentity = std::make_shared<Entity>();


std::vector<const char*> shapetypes = {
						"Box",
						"Sphere",
						"Capsule",
						"Infinite Plane"
};


// Rotation lock
std::vector<const char*> rotations = {
	"RotationX",
	"RotationY",
	"RotationZ"
};

std::vector<const char*> physicsengines = {
	"Box2D",
	"Bullet"
};



// Labels for Options
std::vector<const char*> Label_XYZ = { "X", "Y", "Z" };
std::vector<const char*> Label_RGB = { "R", "G", "B" };

std::vector<const char*> Label_LightTypes = { "Directional", "SpotLight", "PointLight", "No Shadow Point Light"};

glm::vec4 testcolor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

std::unordered_map<std::string, SubTexture> EditorLayer::EditorLayerIcons;

EditorLayer::EditorLayer() {
	ui::windowdatas.push_back(&windowmanagerdata);
	ui::windowdatas.push_back(&scenehierarchywindowdata);
	ui::windowdatas.push_back(&propertieswindowdata);
	ui::windowdatas.push_back(&subtexturewindowdata);
	ui::windowdatas.push_back(&animationcreatorwindowdata);
	ui::windowdatas.push_back(&texturewindowdata);
	ui::windowdatas.push_back(&entityregistrarwindowdata);
	ui::windowdatas.push_back(&skyboxwindowdata);
	ui::windowdatas.push_back(&colorselectorwindowdata);
	ui::windowdatas.push_back(&cameraselectorwindowdata);

	m_depthbuffer = std::make_unique<Texture>(Window::Width, Window::Height, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
	m_colorbuffer = std::make_unique<Texture>(Window::Width, Window::Height, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);


	m_entitybuffer.AddColorAttachment(0, *m_colorbuffer);
	m_entitybuffer.AddDepthAttachment(*m_depthbuffer, GL_DEPTH_STENCIL_ATTACHMENT);

	m_entitycolorshader = std::make_shared<Shader>("res/shaders/entitycolor.vert", "res/shaders/entitycolor.frag");

	// particle generator subtexture
	particlegeneratorsubtexture.index = 15;
	particlegeneratorsubtexture.coords1 = { 0.0f, 1.0f };
	particlegeneratorsubtexture.coords2 = { 0.125f, 0.875f };

	selectedentity->id = (entt::entity)0;

	EditorLayerIconsTexture = std::make_shared<Texture>("res/EditorLayerIcons.png");
	Renderer2D::SetTexture(EditorLayerIconsTexture, 13);


	EditorLayerIcons["ObjectGizmos"] = { 13, {0.0f, 1.0f}, {0.125f, 0.875f} };
	EditorLayerIcons["ColliderGizmos"] = { 13, {0.125f, 1.0f}, {0.25f, 0.875f} };
	EditorLayerIcons["ColorSelect"] = { 13, {0.25f, 1.0f}, {0.375f, 0.875f} };

	ui::selectedcolor = &testcolor;


	// for ui
	ui::cliprects.reserve(16);
	ui::quadindexes.reserve(16);
}


void EditorLayer::RenderEntityHierarchyTree(std::shared_ptr<Entity> entity, unsigned int offset) {

	auto& namec = entity->GetComponent<NameComponent>();
	auto& rc = entity->GetComponent<RelationshipComponent>();


	if (ui::DropDownButton(namec.name.c_str(), rc.isUIOpen, offset)) {
		if (glfwGetKey(Window::ID, GLFW_KEY_C)) {
			auto ce = entity->CreateChildEntity("Entity");
			Scene::s_msgbatch.Add(MessageType::info, "Created Child Entity.");
		}
		else if (glfwGetKey(Window::ID, GLFW_KEY_LEFT_ALT)) {
			Scene::s_msgbatch.Add(MessageType::info, "Entity Deleted.");

			if (entity->HasComponent<RelationshipComponent>())
				std::cout << "Relationcomponent have" << std::endl;

			deleteentity->id = (entt::entity)entity->id;
			deleteentity->m_scene = entity->m_scene;

			if (entity->id == selectedentity->id) {
				selectedentity->id = (entt::entity)0;
				selectedentity->m_scene = nullptr;
			}

			s_GizmoMode = ObjectMode;
		}
		else {
			selectedentity->id = entity->id;
			selectedentity->m_scene = entity->m_scene;
			ui::resetInputs();
		}
	}

	if (rc.isUIOpen) {
		for (auto e : rc.childEntities) {
			RenderEntityHierarchyTree(e, offset + 5);
		}
	}

}



//for vector 3 input box
void vector3f(const std::string& parentid, const std::vector<const char*>& labels, glm::vec3& values, float onclickvalue = 0.0f) {
	for (int i = 0; i < 3; i++) {
		ui::Label(labels[i]);

		std::string id = parentid + labels[i];
		ui::FloatInputBox(id.c_str(), values[i], onclickvalue);
	}
}

void vector4f(const std::string& parentid, const std::vector<const char*>& labels, glm::vec4& values, float onclickvalue = 0.0f) {
	for (int i = 0; i < 4; i++) {
		ui::Label(labels[i]);

		std::string id = parentid + labels[i];
		ui::FloatInputBox(id.c_str(), values[i], onclickvalue);
	}
}




// for selecting entity by clicking on scene
// this is done by reading values from framebuffer color
// color represents entity id
void EditorLayer::SelectEntity() {

	m_entitybuffer.Bind();

	unsigned int mousex = (unsigned int)Window::Window_Mousex;
	unsigned int mousey = (unsigned int)Window::Window_Mousey;

	mousey = Window::Height - mousey;

	unsigned char data[3];

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(mousex, mousey, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, data);

	unsigned int n = (unsigned int)data[0] | (unsigned int)(data[1] << 8) | (unsigned int)(data[2] << 16);


	selectedentity->id = (entt::entity)n;
	selectedentity->m_scene = selectedscene;

	s_GizmoMode = ObjectMode;

	if (!selectedentity->HasComponent<NameComponent>()) {
		selectedentity->m_scene = nullptr;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ui::activeelementid = 0;
}


bool EditorLayer::OnEvent(Event& e) {

	if (ui::onevent(e))
		return true;

	if (test_console.OnEvent(e))
		return true;

	KeyPressedEvent* ke = (KeyPressedEvent*)&e;

	if (ke->mod == GLFW_MOD_CONTROL && ke->key == KEY_S) {
		s_EntityEditingMode = EntityEditingMode::SCALE;
		camera_locked = true;
		return true;
	}

	else if (ke->mod == GLFW_MOD_CONTROL && ke->key == KEY_T) {
		s_EntityEditingMode = EntityEditingMode::TRANSLATE;
		camera_locked = true;
		return true;
	}
	else if (ke->mod == GLFW_MOD_CONTROL && ke->key == KEY_R) {
		s_EntityEditingMode = EntityEditingMode::ROTATE;
		camera_locked = true;
		return true;
	}
	else if (ke->mod == GLFW_MOD_CONTROL && ke->key == KEY_O) {
		showobjectselectionscene = !showobjectselectionscene;
		Application::captureshadowmap = true;
		return true;
	}
	else if (ke->mod == GLFW_MOD_CONTROL && ke->key == KEY_C) {
		CopyEntity();
		Scene::s_msgbatch.Add(MessageType::info, "Entity Copied.");
		return true;
	}
	else if (ke->mod == GLFW_MOD_CONTROL && ke->key == KEY_V) {
		PasteEntity();
		Scene::s_msgbatch.Add(MessageType::info, "Entity Pasted.");
		return true;
	}
	else if (ke->mod == GLFW_MOD_CONTROL && ke->key == KEY_A) {
		m_EntityRegistrar.AddEntity(selectedentity);
		Scene::s_msgbatch.Add(MessageType::info, "Entity Added into Registrar.");
		return true;
	}
	else if (ke->mod == GLFW_MOD_ALT) {
		if (ke->key == KEY_2) {
			Scene::s_msgbatch.Add(MessageType::info, "Camera Move Type : 2-Axis");
			s_EditorCameraType = TWO_AXIS_MOVE_CAMERA;
		}
		else if (ke->key == KEY_3) {
			Scene::s_msgbatch.Add(MessageType::info, "Camera Move Type : 3-Axis");
			s_EditorCameraType = THREE_AXIS_MOVE_CAMERA;
		}
	}

	else if (e.is(Event_Type::Mouse_Scrolled) && !camera_locked) {
		MouseScrolledEvent* ms = (MouseScrolledEvent*)&e;

		if (Input::KeyPressed(KEY_U)) {
			if (ms->yoffset < 0) {
				Window::UI_Size *= 0.95f;
			}
			else {
				Window::UI_Size *= 1.05f;
			}
			return true;
		}


		float speed = 0.01f;
		glm::mat4 cameratransform = glm::mat4(1.0f);
		auto view = selectedscene->m_registry.view<CameraComponent>();
		for (auto e : view) {
			auto& cc = view.get<CameraComponent>(e);
			if (cc.Primary) {
				if (ms->yoffset < 0)
					cc.camerao.orthographicsize += -(ms->yoffset - 1);
				else if (ms->yoffset > 0)
					cc.camerao.orthographicsize -= (ms->yoffset + 1);

				if (cc.camerao.orthographicsize < 0.5f)
					cc.camerao.orthographicsize = 0.5f;
				Window::OrthographicSize = cc.camerao.orthographicsize;
				cc.camerao.Update();
				break;
			}
		}
	}


	return false;
}


void EditorLayer::imguizmos() {
	// for rendering imguizmo

	if (selectedentity->m_scene != nullptr && selectedentity->m_scene->PrimaryCameraExist()) {
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

		ImGui::Begin("gizmos", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
		ImGuizmo::BeginFrame();
		ImGuizmo::SetDrawlist();

		float width = ImGui::GetIO().DisplaySize.x;
		float height = ImGui::GetIO().DisplaySize.y;
		ImGuizmo::SetRect(0.0f, 0.0f, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
		ImGuizmo::Enable(true);

		std::shared_ptr<Entity> cameraentity = selectedscene->GetPrimaryCameraEntity();
		auto& cameracp = cameraentity->GetComponent<CameraComponent>();
		auto& ctc = cameraentity->GetComponent<TransformComponent>();
		glm::mat4 cameraprojection;

		auto parententity = selectedentity->GetComponent<RelationshipComponent>().parentEntity;
		glm::mat4 parentworld = glm::mat4(1.0f);

		if (parententity) {
			parentworld = parententity->GetComponent<TransformComponent>().worldtransform;
		}

		if (cameracp.cameratype == CameraType::Orthographic) {
			cameraprojection = cameracp.camerao.viewprojection;
			ImGuizmo::SetOrthographic(true);
		}
		else {
			cameraprojection = cameracp.camerap.viewprojection;
			ImGuizmo::SetOrthographic(false);
		}

		glm::mat4 cameraview = glm::mat4(1.0f);

		if (cameracp.cameratype == CameraType::Orthographic) {
			cameraview = glm::inverse(ctc.worldtransform);
		}
		else if (cameracp.cameratype == CameraType::Perspective) {
			glm::vec3 pos = glm::vec3(ctc.worldtransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			cameraview = glm::lookAt(pos, pos + Scene::cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		auto& tc = selectedentity->GetComponent<TransformComponent>();
		glm::mat4 transform = glm::mat4(1.0f);

		ImGuizmo::OPERATION op;

		if (s_GizmoMode == ObjectMode) {
			if (s_EntityEditingMode == EntityEditingMode::TRANSLATE)
				op = ImGuizmo::OPERATION::TRANSLATE;
			else if (s_EntityEditingMode == EntityEditingMode::SCALE)
				op = ImGuizmo::OPERATION::SCALE;
			else if (s_EntityEditingMode == EntityEditingMode::ROTATE)
				op = ImGuizmo::OPERATION::ROTATE;

			// Use only worldtransform (no * tc.getmatrix())
			transform = tc.worldtransform;
		}
		else if (s_GizmoMode == ColliderMode) {
			op = ImGuizmo::OPERATION::SCALE;

			auto ttc = selectedentity->GetComponent<TransformComponent>();
			auto& pc = selectedentity->GetComponent<PhysicsComponent>();
			
			ttc.scale = pc.BoxHalfExtents * 2.0f;

			// Use only worldtransform (no * tc.getmatrix())
			transform = parentworld * ttc.getmatrix();
		}

		ImGuizmo::Manipulate(glm::value_ptr(cameraview), glm::value_ptr(cameraprojection), op, ImGuizmo::LOCAL, glm::value_ptr(transform));

		if (ImGuizmo::IsUsing()) {
			Application::captureshadowmap = true;
			was_mousepressed = true;

			Application::GetInstance().s_serializer.m_scene->UpdateLightStatus(tc.position);

            // transform is now the new world transform from gizmo
            // compute new local transform: inverse(parent_world) * new_world_transform

			glm::mat4 parentworld = glm::mat4(1.0f);
			auto parententity = selectedentity->GetComponent<RelationshipComponent>().parentEntity;

			if (parententity) {
				parentworld = parententity->GetComponent<TransformComponent>().worldtransform;
			}

            glm::mat4 newlocal = glm::inverse(parentworld) * transform;

            glm::vec3 position, rotation, scale;
            Math::DecomposeTransform(newlocal, position, rotation, scale);

            glm::vec3 angles = rotation * 180.0f / 3.14f;

            if (s_GizmoMode == ObjectMode) {
				tc.SetRotationEuler(angles);
                tc.position = position;
                tc.scale = scale;
            }
            else if (s_GizmoMode == ColliderMode) {
                auto& pc = selectedentity->GetComponent<PhysicsComponent>();
                pc.BoxHalfExtents = scale / 2.0f;
            }

			ui::resetInputs();
        }

		ImGui::End();
	}

}

void EditorLayer::Draw(float ts) {

	char strid[128];

	// if window resized then change framebuffer size which is used for entity selection

	if (m_colorbuffer->width != Window::Width || m_colorbuffer->height != Window::Height) {


		m_depthbuffer.reset();
		m_colorbuffer.reset();

		
		m_depthbuffer = std::make_unique<Texture>(Window::Width, Window::Height, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
		m_colorbuffer = std::make_unique<Texture>(Window::Width, Window::Height, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);

		m_entitybuffer.AddColorAttachment(0, *m_colorbuffer);
		m_entitybuffer.AddDepthAttachment(*m_depthbuffer, GL_DEPTH_STENCIL_ATTACHMENT);
	}

	// handle window moving
	//ui::windowmove();

	// calculate values used for ui element rendering
	//ui::calculatevalues();

	// check mouse hovered on window
	for (auto data : ui::windowdatas) {
		if (data->visible) {
			if (ui::mousehover(data->x, data->y, data->width, data->content_offset_y)) {
				mouse_on_window = true;
			}
		}
	}


	// Window manager window always visible
	windowmanagerdata.visible = true;

	//float ui_ortho = Window::OrthographicSize / 2;

	//glm::mat4 orthomatrix = glm::ortho(-ui_ortho * Window::Ratio, ui_ortho * Window::Ratio, -ui_ortho, ui_ortho);
	glm::mat4 orthomatrix = glm::ortho(0.0f, (float)Window::Width, (float)Window::Height, 0.0f);

	Renderer2D::UseShader(ShaderManager::Get("2d"));

	Renderer2D::BeginScene(orthomatrix);

	// Window Manager Window
	{
		ui::Begin("Window Manager", windowmanagerdata);
		ui::pushstyle(uistyle::row_two_block);

		if (ui::Button("Scene Hierarchy")) {
			scenehierarchywindowdata.visible = true;
		}

		if (ui::Button("Properties")) {
			propertieswindowdata.visible = true;
		}

		if (ui::Button("Subtexture")) {
			subtexturewindowdata.visible = true;
		}

		if (ui::Button("Animation")) {
			animationcreatorwindowdata.visible = true;
		}

		if (ui::Button("Textures")) {
			texturewindowdata.visible = true;
		}

		if (ui::Button("EntityRegistrar")) {
			entityregistrarwindowdata.visible = true;
		}

		if (ui::Button("SkyBox")) {
			skyboxwindowdata.visible = true;
		}

        if(ui::Button("colorselect")) {
          colorselectorwindowdata.visible = true;
        }

		if (ui::Button("cameraselector")) {
			cameraselectorwindowdata.visible = true;
		}


		ui::Label("EditorCameraMoveSpeed");
		ui::FloatInputBox("editorcameramovespeed", EditorCameraSpeed, 0.0f);

		ui::Label("Uniform Ambient");
		ui::FloatInputBox("worldambient", selectedscene->ambientscale);

		ui::popstyle();

		unsigned int op = selectedscene->m_physicsType;
		ui::OptionSelector(physicsengines, op);
		selectedscene->m_physicsType = (PhysicsType)op;

		unsigned int routput = Application::s_renderoutput;
		ui::OptionSelector(renderoutputstr, routput);
		Application::s_renderoutput = (RenderOutput)routput;

		if (ui::Button("Reorder Rendering Sequence")) {
			selectedscene->reorder_rendering_sequence();
		}

		ui::CheckBox("Show Collider", EditorLayer::showcolliders);

		ui::Separator();

		if (ui::Button("Save Scene")) {
			auto app = Application::GetInstance();
			app.s_serializer.Serialize(app.s_serializer.filepath);
			Scene::s_msgbatch.Add(MessageType::info, "Scene Saved.", 1.0f);
		}

		ui::Separator();

		sprintf(strid, "Time : %f ms", ts * 1000.0f);
		ui::Label(strid);
		ui::End();

	}


    // color selector window
    {
      ui::Begin("Color Picker", colorselectorwindowdata);
      ui::ColorPicker();
      ui::End();
    }


	// Scene Hierarchy Window
	{
		ui::Begin("Scene Hierarchy", scenehierarchywindowdata);

		auto group = selectedscene->m_registry.group<NameComponent>();

		for (auto e : group) {
			std::shared_ptr<Entity> entity = std::make_shared<Entity>();
			entity->id = e;
			entity->m_scene = selectedscene;

			auto& rc = entity->GetComponent<RelationshipComponent>();
			if (rc.parentEntity == nullptr) {

				RenderEntityHierarchyTree(entity, 0);

			}
		}
		ui::Separator();
		if (ui::Button("Add Entity")) {
			selectedscene->CreateEntity("Entity");
			Scene::s_msgbatch.Add(MessageType::info, "Entity Added.");
		}
		ui::End();
	}

	// Properties Window
	{
		if (selectedentity->m_scene != nullptr) {
			ui::Begin("Properties", propertieswindowdata);
			ui::pushstyle(uistyle::row_two_block);

			//auto& cachedcomponent = selectedentity->GetComponent<CachedComponents>();

			// NameComponent
			{

				ui::Label("Name");
				auto& namec = selectedentity->GetComponent<NameComponent>();
				ui::InputBox("entityname", namec.name);

				ui::Separator();
			}

			// TransformComponent
			{

				auto& transform = selectedentity->GetComponent<TransformComponent>();
				// for position
				ui::StateButton("Position", position, false);
				ui::StateButton("(Vector3)", position, true);

				if (position) {
					vector3f("entityposition", positionvaluesstring, transform.position);

				}
				// for Rotation
				ui::StateButton("Rotation", rotation, false);
				ui::StateButton("(Vector3)", rotation, true);

				if (rotation)
					vector3f("entityrotation", positionvaluesstring, transform.rotation);

				transform.SetRotationEuler(transform.rotation);

				// for scale
				ui::StateButton("Scale", scale, false);
				ui::StateButton("(Vector3)", scale, true);

				if (scale)
					vector3f("entityscale", positionvaluesstring, transform.scale, 1.0f);
				ui::popstyle();

			}


			// FontRendererComponent
			{
				if (selectedentity->HasComponent<FontRendererComponent>()) {
					auto& frc = selectedentity->GetComponent<FontRendererComponent>();

					ui::Separator();
					ui::pushstyle(uistyle::row_two_block);

					ui::Label("Text");
					ui::InputBox("fontrenderer_text", frc.text);

					ui::Label("Opacity");
					ui::FloatInputBox("fontrenderer_opacity", frc.opacity);

					ui::Label("Size");
					ui::FloatInputBox("fontrenderer_fontsize", frc.pixelsize);

					ui::popstyle();

					if (ui::Button("Delete FontRendererComponent")) {
						selectedentity->RemoveComponent<FontRendererComponent>();
					}
				}
			}

			// UIFontRendererComponent
			{
				if (selectedentity->HasComponent<UIFontRendererComponent>()) {
					auto& frc = selectedentity->GetComponent<UIFontRendererComponent>();

					ui::Separator();
					ui::pushstyle(uistyle::row_two_block);

					ui::Label("Text");
					ui::InputBox("fontrenderer_text", frc.text);

					ui::Label("Opacity");
					ui::FloatInputBox("fontrenderer_opacity", frc.opacity);

					ui::Label("Size");
					ui::FloatInputBox("fontrenderer_fontsize", frc.pixelsize);

					ui::popstyle();

					if (ui::Button("Delete UIFontRendererComponent")) {
						selectedentity->RemoveComponent<UIFontRendererComponent>();
					}
				}
			}

			// SpriteRendererComponent
			{
				if (selectedentity->HasComponent<SpriteRendererComponent>()) {
					auto& spc = selectedentity->GetComponent<SpriteRendererComponent>();

					ui::Separator();

					std::vector<const char*> colororsubtexture = { "Color", "SubTexture" };

					spriterenderertypeindex = spc.type;
					ui::OptionSelector(colororsubtexture, spriterenderertypeindex);
					spc.type = spriterenderertypeindex;

					ui::CheckBox("Transparent", spc.transparent);

					if (spc.type != 0) {
						if (ui::Button("SubTexture")) {
							selectedsubtexture = &spc.m_subtexture;
						}
					}

					else {
						ui::pushstyle(uistyle::row_two_block);

						//vector4f("subtexturecolor", colorvaluesstring, spc.color, 0.0f);
						ui::Label("Color");
						ui::ColorEdit(spc.color);

						ui::popstyle();

					}



					if (ui::Button("Delete SpriteRendererComponent")) {
						selectedentity->RemoveComponent<SpriteRendererComponent>();
					}
				}
			}


			// UISpriteRendererComponent
			{
				if (selectedentity->HasComponent<UISpriteRendererComponent>()) {
					auto& spc = selectedentity->GetComponent<UISpriteRendererComponent>();

					ui::Separator();

					std::vector<const char*> colororsubtexture = { "Color", "SubTexture" };

					spriterenderertypeindex = spc.type;
					ui::OptionSelector(colororsubtexture, spriterenderertypeindex);
					spc.type = spriterenderertypeindex;

					ui::CheckBox("Transparent", spc.transparent);

					if (spc.type != 0) {
						if (ui::Button("SubTexture")) {
							selectedsubtexture = &spc.m_subtexture;
						}
					}

					else {
						ui::pushstyle(uistyle::row_two_block);

						//vector4f("subtexturecolor", colorvaluesstring, spc.color, 0.0f);
						ui::Label("Color");
						ui::ColorEdit(spc.color);

						ui::popstyle();

					}



					if (ui::Button("Delete UISpriteRendererComponent")) {
						selectedentity->RemoveComponent<UISpriteRendererComponent>();
					}
				}
			}


			// CameraComponent
			{
				if (selectedentity->HasComponent<CameraComponent>()) {
					ui::pushstyle(uistyle::row_two_block);
					ui::Separator();

					ui::StateButton("Camera", cameraprop, false);
					ui::StateButton("Show", cameraprop, true);

					if (cameraprop) {
						auto& cc = selectedentity->GetComponent<CameraComponent>();

						if (cc.cameratype == CameraType::Orthographic) {
							ui::Label("Near");
							ui::FloatInputBox("camera_near", cc.camerao.near);

							ui::Label("Far");
							ui::FloatInputBox("camera_far", cc.camerao.far);


							ui::Label("Size");
							ui::FloatInputBox("camera_size", cc.camerao.orthographicsize);

						}

						else if (cc.cameratype == CameraType::Perspective) {
							ui::Label("Near");
							ui::FloatInputBox("camera_near", cc.camerap.near);

							ui::Label("Far");
							ui::FloatInputBox("camera_far", cc.camerap.far);
						}

						ui::Label("Min X");
						ui::FloatInputBox("cameraminx", cc.x_min);

						ui::Label("Max X");
						ui::FloatInputBox("cameramaxx", cc.x_max);

						ui::Label("Min Y");
						ui::FloatInputBox("cameraminy", cc.y_min);

						ui::Label("Max Y");
						ui::FloatInputBox("cameramaxy", cc.y_max);

						float temp = cc.camerao.orthographicsize;
						static bool pers = false;
						if (ui::CheckBox("perspective", pers)) {
							cc.cameratype = CameraType::Perspective;
						}
						else
							cc.cameratype = CameraType::Orthographic;

						if (temp != cc.camerao.orthographicsize)
							cc.camerao.Update();


						if (ui::CheckBox("Primary", cc.Primary)) {
							auto g2 = selectedscene->m_registry.view<CameraComponent>();
							for (auto e : g2) {
								auto& cmc = g2.get<CameraComponent>(e);
								cmc.Primary = false;
							}
							cc.Primary = true;
						}
					}
					ui::popstyle();

					if (ui::Button("Delete CameraComponent")) {
						selectedentity->RemoveComponent<CameraComponent>();
					}
				}
			}


			// ScriptComponent
			{
				if (selectedentity->HasComponent<ScriptComponent>()) {
					auto& scriptc = selectedentity->GetComponent<ScriptComponent>();
					ui::pushstyle(uistyle::row_two_block);
					ui::Separator();
					ui::Label("Script");
					ui::InputBox("script_name", scriptc.scriptname);
					ui::popstyle();

					if (ui::Button("Load Script")) {
						scriptc.destroy();
					}

					if (ui::Button("Delete ScriptComponent")) {
						selectedentity->RemoveComponent<ScriptComponent>();
					}
				}
			}

			// RigidBody2DComponent
			{
				if (selectedentity->HasComponent<RigidBody2DComponent>()) {
					auto& rb2d = selectedentity->GetComponent<RigidBody2DComponent>();
					ui::Separator();
					ui::pushstyle(uistyle::row_two_block);

					if (rb2d.type == b2BodyType::b2_staticBody)
						rigidbodytypeindex = 0;
					else
						rigidbodytypeindex = 1;

					ui::Label("Type");
					ui::OptionSelector(rigidbodytypes, rigidbodytypeindex);

					if (rigidbodytypeindex == 0)
						rb2d.type = b2BodyType::b2_staticBody;
					else if (rigidbodytypeindex == 1)
						rb2d.type = b2BodyType::b2_dynamicBody;

					ui::Label("id");
					ui::UIntInputBox("rigidbodyid", rb2d.id);

					ui::CheckBox("FixedRotation", rb2d.fixedrotation);
					ui::popstyle();

					if (ui::Button("Delete RigidBody2DComponent")) {
						selectedentity->RemoveComponent<RigidBody2DComponent>();
					}
				}
			}

			// BoxCollider2DComponent
			{
				if (selectedentity->HasComponent<BoxCollider2DComponent>()) {
					auto& bc2d = selectedentity->GetComponent<BoxCollider2DComponent>();
					ui::Separator();

					ui::pushstyle(uistyle::row_two_block);

					ui::Label("Density");
					ui::FloatInputBox("density", bc2d.density);
					ui::Label("Friction");
					ui::FloatInputBox("friction", bc2d.friction);
					ui::Label("Width");
					ui::FloatInputBox("colliderwidth", bc2d.width);
					ui::Label("Height");
					ui::FloatInputBox("colliderheight", bc2d.height);
					ui::Label("ObjectID");
					ui::UIntInputBox("boxshapecolliderid", bc2d.objectid, 0);

					ui::CheckBox("isSensor", bc2d.isSensor);
					ui::CheckBox("SameAsScale", bc2d.sameasscale);
					ui::popstyle();

					if (ui::Button("Delete BoxCollider2DComponent")) {
						selectedentity->RemoveComponent<BoxCollider2DComponent>();
					}
				}
			}

			// 2D AnimationGroupComponent
			{
				if (selectedentity->HasComponent<AnimationGroupComponent>()) {
					auto& agcp = selectedentity->GetComponent<AnimationGroupComponent>();

					ui::Separator();

					ui::pushstyle(uistyle::row_two_block);

					for (unsigned int i = 0; i < agcp.animationpath.size(); i++) {
						sprintf(strid, "aniimationid%d", i);
						ui::InputBox(strid, agcp.animationpath[i].first);
						sprintf(strid, "animationpath%d", i);
						ui::InputBox(strid, agcp.animationpath[i].second);
					}

					ui::popstyle();

					if (ui::Button("Add Animation")) {
						agcp.addanimationpath("", "");
					}

					if (ui::Button("Load Animations")) {
						agcp.loadanimations();
					}

					if (ui::Button("Delete AnimationGroupComponent")) {
						selectedentity->RemoveComponent<AnimationGroupComponent>();
					}
				}
			}

			// 2D ParticleGeneratorComponent
			{
				if (selectedentity->HasComponent<ParticleGeneratorComponent>()) {
					auto& pg = selectedentity->GetComponent<ParticleGeneratorComponent>();
					auto& tc = selectedentity->GetComponent<TransformComponent>();


					pg.m_particleprops.scale = { tc.scale.x, tc.scale.y };
					pg.m_particleprops.z_rotation = tc.rotation.z;

					if (ui::Button("SubTexture")) {
						selectedsubtexture = &pg.m_particleprops.m_subtexture;
					}
					ui::pushstyle(uistyle::row_two_block);

					ui::Label("Particle / Second");
					ui::UIntInputBox("particlecount", pg.count, 0);

					ui::Label("Velocity.x");
					ui::FloatInputBox("Velocity.x", pg.m_particleprops.velocity.x);
					ui::Label("Velocity.y");
					ui::FloatInputBox("Velocity.y", pg.m_particleprops.velocity.y);
					ui::Label("Velocity.z");
					ui::FloatInputBox("Velocity.z", pg.m_particleprops.velocity.z);

					ui::Label("VelocityVariation.x");
					ui::FloatInputBox("VelocityVariation.x", pg.m_particleprops.velocityvariation.x);
					ui::Label("VelocityVariation.y");
					ui::FloatInputBox("VelocityVariation.y", pg.m_particleprops.velocityvariation.y);
					ui::Label("VelocityVariation.z");
					ui::FloatInputBox("VelocityVariation.z", pg.m_particleprops.velocityvariation.z);

					ui::Label("Gravity");
					ui::CheckBox("ParticleGravity", pg.m_particleprops.Gravity);

					ui::Label("BeginSize");
					ui::FloatInputBox("BeginSize", pg.m_particleprops.sizebegin);
					ui::Label("EndSize");
					ui::FloatInputBox("EndSize", pg.m_particleprops.sizeend);

					ui::Label("LifeTime");
					ui::FloatInputBox("LifeTime", pg.m_particleprops.lifetime);

					ui::Label("SizeVariation");
					ui::FloatInputBox("sizevariation", pg.m_particleprops.sizevariation);

					ui::popstyle();

					ui::CheckBox("Generate_From_Area", pg.m_particleprops.generate_from_area);

					ui::CheckBox("Active", pg.active);

					ui::pushstyle(uistyle::row_two_block);
	
					ui::Label("ColorBegin");
					ui::ColorEdit(pg.m_particleprops.colorbegin);

					ui::Label("ColorEnd");
					ui::ColorEdit(pg.m_particleprops.colorend);

					
					ui::popstyle();

					if (ui::Button("Delete ParticleGeneratorComponent")) {
						selectedentity->RemoveComponent<ParticleGeneratorComponent>();
					}
				}
			}

			// Model3DComponent
			{
				if (selectedentity->HasComponent<Model3DComponent>()) {
					ui::Separator();

					auto& mc = selectedentity->GetComponent<Model3DComponent>();

					ui::pushstyle(uistyle::row_two_block);

					ui::Label("File name");
					ui::InputBox("3dmodelfileinput", mc.filepath);

					ui::popstyle();

					ui::CheckBox("Bloom", mc.bloom);
					ui::CheckBox("Outline", mc.outline);

					if (ui::Button("Load Model")) {
						mc.mModel = ModelManager::GetModel(mc.filepath);
					}

					if (ui::Button("Remove ModelComponent")) {
						selectedentity->RemoveComponent<Model3DComponent>();
					}
				}
			}

			// LightComponent
			{
				HZ_PROFILE_SCOPE("LightComponent");
				if (selectedentity->HasComponent<LightComponent>()) {
					ui::Separator();

					auto& lc = selectedentity->GetComponent<LightComponent>();

					ui::pushstyle(uistyle::row_two_block);

					ui::Label("LightType");
					unsigned int lighttypes = lc.lighttype;
					ui::OptionSelector(Label_LightTypes, lighttypes);

					if (lighttypes != lc.lighttype) {
						lc.m_Texture.reset();
						Application::captureshadowmap = true;
					}

					lc.lighttype = (LightType)lighttypes;

					ui::popstyle();

					ui::Label("Light Color");

					ui::pushstyle(uistyle::row_two_block);
					//vector3f("LightComponentColor", Label_RGB, lc.color);
					ui::Label("LightColor");
					ui::ColorEdit(lc.color);

					if (lc.lighttype != POINT_LIGHT) {
						ui::Separator();
						ui::popstyle();
						ui::Label("Light Direction");
						ui::pushstyle(uistyle::row_two_block);
						vector3f("LightComponentDirection", Label_XYZ, lc.direction);
					}

					if (lc.lighttype == SPOT_LIGHT) {
						ui::Label("Angle");
						ui::FloatInputBox("spotlightangle", lc.angle);
					}

					ui::popstyle();

					if (ui::Button("Remove LightComponent")) {
						selectedentity->RemoveComponent<LightComponent>();
					}

				}
			}

			// I Created for monopoly Game
			// CardComponent
			{
				if (selectedentity->HasComponent<CardComponent>()) {
					ui::Separator();

					ui::pushstyle(uistyle::row_two_block);
					ui::Label("Card Type ID");

					auto& ctype = selectedentity->GetComponent<CardComponent>();

					unsigned int id = (unsigned int)ctype.m_Cardtype;
					ui::UIntInputBox("sitetypeid", id);
					ctype.m_Cardtype = (CARDTYPE)id;

					ui::popstyle();

					if (ui::Button("Delete CardComponent")) {
						selectedentity->RemoveComponent<CardComponent>();
					}
				}
			}

			// PhysicsComponent
			{
				if (selectedentity->HasComponent<PhysicsComponent>()) {
					ui::Separator();

					ui::pushstyle(uistyle::row_two_block);

					auto& pc = selectedentity->GetComponent<PhysicsComponent>();

					unsigned int index = (unsigned int)pc.ShapeType;

					static const char* shapestr = "Shape";

					ui::Label(shapestr);
					ui::OptionSelector(shapetypes, index);
					pc.ShapeType = (PhysicsShapeType)index;


					static const char* massstr = "Mass";
					static const char* Bulletmass = "BulletMass";
					static const char* halfextentstr = "Halfextent3";
					static const char* sphereradiusstr = "Sphere Radius";
					static const char* bulletradius = "bulletsphereradius";


					ui::Label(massstr);
					ui::FloatInputBox(Bulletmass, pc.Mass);

					if (pc.ShapeType == PhysicsShapeType::Box || pc.ShapeType == PhysicsShapeType::Capsule) {
						vector3f(halfextentstr, Label_XYZ, pc.BoxHalfExtents);
					}

					if (pc.ShapeType == PhysicsShapeType::Capsule || pc.ShapeType == PhysicsShapeType::Sphere) {
						ui::Label(sphereradiusstr);
						ui::FloatInputBox(bulletradius, pc.SphereRadius);
					}

					static const char* restitutionstr = "Restutition";
					static const char* OnlyPOsitiveSide = "OnlyPositiveSide";

					ui::Label(restitutionstr);
					ui::FloatInputBox(restitutionstr, pc.Restitution);

					ui::Label(OnlyPOsitiveSide);
					ui::CheckBox(OnlyPOsitiveSide, pc.OnlyPositive);

					ui::popstyle();

					for (unsigned int i = 0; i < 3; i++) {
						ui::CheckBox(rotations[i], pc.Rotations[i]);
					}

					if (ui::Button("Remove Physics Component")) {
						selectedentity->RemoveComponent<PhysicsComponent>();
					}
				}
			}

			// Animation3DComponent
			{
				if (selectedentity->HasComponent<Animation3DComponent>()) {

					auto& ac = selectedentity->GetComponent<Animation3DComponent>();

					unsigned int len = ac.mAnimationNames.size();

					for (unsigned int i = 0; i < len; i++) {
						bool check = ac.activeanimatioins[i];
						ui::CheckBox(ac.mAnimationNames[i].c_str(), check);
						if (check != ac.activeanimatioins[i]) {
							ac.setActive(i);
						}
					}

					if (ui::Button("Reload")) {
						if (selectedentity->HasComponent<Model3DComponent>()) {
							auto& mc = selectedentity->GetComponent<Model3DComponent>();
							ac.Reload(mc.filepath, mc.mModel.get());
						}
					}

					if (ui::Button("Remove Animation3D Component")) {
						selectedentity->RemoveComponent<Animation3DComponent>();
					}
				}
			}

			ui::Separator();

			// for adding other components
			{

				if (!selectedentity->HasComponent<CameraComponent>()) {
					if (ui::Button("Camera Component")) {
						selectedentity->AddComponent<CameraComponent>();
					}
				}

				if (!selectedentity->HasComponent<SpriteRendererComponent>()) {
					if (ui::Button("Sprite Component")) {
						selectedentity->AddComponent<SpriteRendererComponent>();
					}
				}

				if (!selectedentity->HasComponent<UISpriteRendererComponent>()) {
					if (ui::Button("UISprite Component")) {
						selectedentity->AddComponent<UISpriteRendererComponent>();
					}
				}

				if (!selectedentity->HasComponent<FontRendererComponent>()) {
					if (ui::Button("Fontrenderer Component")) {
						selectedentity->AddComponent<FontRendererComponent>("", 14.0f);
					}
				}

				if (!selectedentity->HasComponent<UIFontRendererComponent>()) {
					if (ui::Button("UIFontRenderer Component")) {
						selectedentity->AddComponent<UIFontRendererComponent>("", 14.0f);
					}
				}

				if (!selectedentity->HasComponent<ScriptComponent>()) {
					if (ui::Button("Script Component")) {
						selectedentity->AddComponent<ScriptComponent>("");
					}
				}

				if (!selectedentity->HasComponent<RigidBody2DComponent>()) {
					if (ui::Button("RigidBody2D Component")) {
						selectedentity->AddComponent<RigidBody2DComponent>();
					}
				}

				if (!selectedentity->HasComponent<BoxCollider2DComponent>()) {
					if (ui::Button("BoxCollider2D Component")) {
						auto transform = selectedentity->GetComponent<TransformComponent>();
						auto& bc = selectedentity->AddComponent<BoxCollider2DComponent>();
						bc.width = transform.scale.x;
						bc.height = transform.scale.y;
					}
				}

				if (!selectedentity->HasComponent<AnimationGroupComponent>()) {
					if (ui::Button("AnimationGroup Component")) {
						selectedentity->AddComponent<AnimationGroupComponent>();
					}
				}

				if (!selectedentity->HasComponent<ParticleGeneratorComponent>()) {
					if (ui::Button("Particle Generator Component")) {
						selectedentity->AddComponent<ParticleGeneratorComponent>();
					}
				}

				if (!selectedentity->HasComponent<Model3DComponent>()) {
					if (ui::Button("Model3DComponent")) {
						selectedentity->AddComponent<Model3DComponent>();
					}
				}

				if (!selectedentity->HasComponent<LightComponent>()) {
					if (ui::Button("LightComponent")) {
						selectedentity->AddComponent<LightComponent>();
						Application::captureshadowmap = true;
					}
				}

				if (!selectedentity->HasComponent<CardComponent>()) {
					if (ui::Button("CardComponent")) {
						selectedentity->AddComponent<CardComponent>();
					}
				}

				if (!selectedentity->HasComponent<PhysicsComponent>()) {
					if (ui::Button("PhysicsComponent")) {
						selectedentity->AddComponent<PhysicsComponent>();
					}
				}

				if (!selectedentity->HasComponent<Animation3DComponent>()) {
					if (ui::Button("Animation3DComponent")) {
						selectedentity->AddComponent<Animation3DComponent>();
					}
				}
			}
			ui::End();
		}
	}

	// SubTexture Window 
	{
		ui::Begin("Subtexture", subtexturewindowdata);

		if (selectedsubtexture) {
			ui::Image(selectedsubtexture);
			ui::Separator();

			ui::pushstyle(uistyle::row_two_block);

			ui::Label("Texture Index");
			ui::UIntInputBox("textureindex", selectedsubtexture->index);

			ui::Label("Coord1.x");
			ui::FloatInputBox("coord1.x", selectedsubtexture->coords1.x);
			ui::Label("Coord1.y");
			ui::FloatInputBox("coord1.y", selectedsubtexture->coords1.y);

			ui::Label("Coord2.x");
			ui::FloatInputBox("coord2.x", selectedsubtexture->coords2.x);
			ui::Label("Coord2.y");
			ui::FloatInputBox("coord2.y", selectedsubtexture->coords2.y);

			ui::Label("Width Multiplier");
			ui::FloatInputBox("WidthMultiplier", selectedsubtexture->X_Multiplier);
			ui::Label("Height Multtiplier");
			ui::FloatInputBox("HeightMultiplier", selectedsubtexture->Y_Multiplier);
			ui::popstyle();
		}

		ui::End();
	}

	// 2D Animation Creator Window
	{
		ui::Begin("Animation Creator", animationcreatorwindowdata);

		ui::pushstyle(uistyle::row_two_block);

		unsigned int framecount = m_animation.frames.size();

		if (framecount > 0) {
			SubTexture abuffer = m_animation.getframe();
			ui::Image(&abuffer);
		}


		ui::Label("Path");
		ui::InputBox("animationpath", animationfilepath);

		for (unsigned int i = 0; i < m_animation.frames.size(); i++) {

			if (ui::Button("Subtexture")) {
				selectedsubtexture = &m_animation.frames[i];
			}
			std::string id = "animationframe" + std::to_string(i);
			ui::FloatInputBox(id.c_str(), m_animation.frametimes[i]);
		}


		if (animationplaying) {
			m_animation.Update(ts / 1000);
			if (ui::Button("Stop")) {
				animationplaying = false;
			}
		}

		else {
			if (ui::Button("Play")) {
				animationplaying = true;
			}
		}

		if (ui::Button("Reset")) {
			m_animation.time = 0.0f;
		}

		ui::popstyle();

		if (ui::Button("Save")) {
			writeanimation(animationfilepath, m_animation);
		}

		if (ui::Button("Load Animation")) {
			m_animation.frames.clear();
			m_animation.frametimes.clear();
			m_animation.time = 0.0f;
			m_animation = readanimation(animationfilepath);
		}

		if (ui::Button("Add Frame")) {
			SubTexture s;
			m_animation.Append(s, 0.0f);
		}


		if (ui::Button("Clear")) {
			m_animation.frames.clear();
			m_animation.frametimes.clear();
			m_animation.time = 0.0f;
		}

		ui::End();
	}

	// Texture Window
	{
		ui::Begin("Textures", texturewindowdata);
		ui::pushstyle(uistyle::row_two_block);
		for (uint32_t i = 2; i < 16; i++) {
			if (!Renderer2D::textures[i]) {
				Renderer2D::textures[i] = std::make_shared<Texture>(1, 1, GL_RGBA8, GL_RGBA, 0);
			}
			std::string number = std::to_string(i);
			ui::Label(number.c_str());
			std::string id = "texture" + number;
			ui::InputBox(id.c_str(), Renderer2D::textures[i]->path);
		}
		ui::popstyle();

		// for reload textures
		if (ui::Button("Reload Texture")) {
			for (unsigned int i = 2; i < 16; i++) {
				if (Renderer2D::textures[i]->path != "") {
					std::string path = Renderer2D::textures[i]->path;
					std::cout << path << std::endl;
					Renderer2D::textures[i].reset();
					Renderer2D::textures[i] = std::make_shared<Texture>(path.c_str());
				}
			}
		}
		ui::End();
	}

	// camera selector
	{
		ui::Begin("CameraSelector", cameraselectorwindowdata);

		auto view = selectedscene->m_registry.view<CameraComponent, NameComponent>();
		for (auto e : view) {

			auto& nc = view.get<NameComponent>(e);
			if (ui::Button(nc.name.c_str())) {

				for (auto e2 : view) {
					auto& cc = view.get<CameraComponent>(e2);
					cc.Primary = false;
				}

				auto& cc = view.get<CameraComponent>(e);
				cc.Primary = true;
			}
 		}

		ui::End();
	}

	// EntityRegistrar Window
	{
		ui::Begin("EntityRegistrar", entityregistrarwindowdata);

		ui::Label("Click On Entity Button To Add Entity in Scene.");

		if (ui::Button("Save")) {
			m_EntityRegistrar.Save();
		}

		ui::Separator();

		ui::pushstyle(uistyle::row_two_block);
		auto entityregistrarview = m_EntityRegistrar.m_scene->m_registry.view<NameComponent>();

		for (auto e : entityregistrarview) {
			auto& nc = entityregistrarview.get<NameComponent>(e);

			std::shared_ptr<Entity> entity = std::make_shared<Entity>();
			entity->id = e;
			entity->m_scene = m_EntityRegistrar.m_scene;

			if (ui::Button(nc.name.c_str())) {
				m_EntityRegistrar.AddEntityIntoScene(selectedscene, entity);
			}

			else if (ui::Button("Remove")) {
				m_EntityRegistrar.m_scene->DestroyEntity(entity);
			}
		}
		ui::popstyle();
		ui::End();
	}

	// SkyBox window
	{
		ui::Begin("SkyBox", skyboxwindowdata);

		ui::pushstyle(uistyle::row_two_block);

		auto& app = Application::GetInstance();

		ui::Label("Right Image");
		ui::InputBox("skyboxrightimage", app.skybox->filepaths[0]);

		ui::Label("Left Image");
		ui::InputBox("skyboxleftimage", app.skybox->filepaths[1]);

		ui::Label("Top Image");
		ui::InputBox("skyboxtopimage", app.skybox->filepaths[2]);

		ui::Label("Bottom Image");
		ui::InputBox("skyboxbottomimage", app.skybox->filepaths[3]);

		ui::Label("Front Image");
		ui::InputBox("skyboxfrontimage", app.skybox->filepaths[4]);

		ui::Label("Back Image");
		ui::InputBox("skyboxbackimage", app.skybox->filepaths[5]);

		ui::popstyle();

		if (ui::Button("Load Textures")) {
			for (unsigned int i = 0; i < 5; i++) {
				app.skybox->AddTexture(i, app.skybox->filepaths[i]);
			}
		}

		ui::End();

	}


	ui::quadindexes.push_back(Renderer2D::state.indicespointer / 6);
	ui::cliprects.push_back(glm::vec4(0.0f, 0.0f, Window::Width, Window::Height));


	// Other UI Stuff
	{
		// testing of image button

		if (selectedentity) {
			if (!selectedentity->HasComponent<PhysicsComponent>()) {
				s_GizmoMode = ObjectMode;
			}

			unsigned int y = Window::Height - 50;
			if (ui::ImageButton({ 0, y }, { 50, 50 }, EditorLayerIcons["ObjectGizmos"], s_GizmoMode == ObjectMode ? 1.0f : 0.5f)) {
				s_GizmoMode = ObjectMode;
			}
			else if (ui::ImageButton({ 50, y }, { 50,50 }, EditorLayerIcons["ColliderGizmos"], s_GizmoMode == ColliderMode ? 1.0f : 0.5f)) {

				if (selectedentity->HasComponent<PhysicsComponent>()) {
					s_GizmoMode = ColliderMode;
				}
			}
		}

	}

	unsigned int len = ui::quadindexes.size();
	for (unsigned int i = len; i < 16; i++) {
		ui::quadindexes.push_back(10000);
		ui::cliprects.push_back(glm::vec4(0.0f));
	}

	auto shader = ShaderManager::Get("2d");

	char buffer[128];
	for (unsigned int i = 0; i < 16; i++) {
		sprintf(buffer, "quadindexes[%d]", i);
		shader->SetInt(buffer, ui::quadindexes[i]);
		sprintf(buffer, "clips[%d]", i);
		shader->SetVec4(buffer, ui::cliprects[i]);
	}

	shader->SetInt("isui", true);

	Renderer2D::EndScene();

	shader->SetInt("isui", false);

	ui::quadindexes.clear();
	ui::cliprects.clear();




	if (deleteentity->m_scene) {
		selectedscene->DestroyEntity(deleteentity);
		deleteentity->m_scene = nullptr;
	}


	// Draw Console 
	{
		Renderer2D::BeginScene(orthomatrix);
		test_console.Draw();
		Renderer2D::EndScene();
	}


	/*
	// for Entity selection
	{
		m_entitybuffer.Bind();
		Renderer2D::UseShader(m_entitycolorshader);

		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 camproj = glm::mat4(1.0f);
		auto view = selectedscene->m_registry.view<CameraComponent, TransformComponent>();
		for (auto e : view) {
			auto& cc = view.get<CameraComponent>(e);
			auto& tc = view.get<TransformComponent>(e);
			if (cc.Primary) {
				if (cc.cameratype == CameraType::Orthographic) {
					cc.camerao.Update();
					camproj = cc.camerao.viewprojection * glm::inverse(tc.getmatrix());
				}
				else {
					cc.camerap.Update();
					camproj = cc.camerap.viewprojection * glm::inverse(tc.getmatrix());
				}
				break;
			}
		}

		// for entity selection framebuffer

		Renderer2D::BeginScene(camproj);


		auto view2 = selectedscene->m_registry.view<SpriteRendererComponent, TransformComponent>();
		for (auto e : view2) {
			auto& sc = view2.get<SpriteRendererComponent>(e);
			auto& tc = view2.get<TransformComponent>(e);

			uint32_t n = (uint32_t)e;
			Renderer2D::DrawEntity((float)n, tc.position, tc.scale, tc.rotation.z, sc.m_subtexture);
		}

		auto view3 = selectedscene->m_registry.view<ParticleGeneratorComponent, TransformComponent>();
		for (auto e : view3) {
			auto& tc = view3.get<TransformComponent>(e);

			uint32_t n = (uint32_t)e;
			Renderer2D::DrawEntity((float)n, tc.position, { tc.scale.x, tc.scale.y }, 0.0f, particlegeneratorsubtexture);
		}

		Renderer2D::EndScene();

	}

	*/

	
	if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1)) {
		was_mousepressed = true;
		ui::was_mouse_pressed = true;
	}
	else {
		was_mousepressed = false;
		ui::was_mouse_pressed = false;
	}

	mouse_on_window = false;
}

void EditorLayer::CopyEntity() {
	if (selectedentity->m_scene != nullptr) {

		// copy namecomponent
		NameComponent nc = selectedentity->GetComponent<NameComponent>();
		s_CopiedEntity.nc = nc;

		// copy transformcomponent
		TransformComponent tc = selectedentity->GetComponent<TransformComponent>();
		s_CopiedEntity.tc = tc;

		// copy cameracomponent
		s_CopiedEntity.cc.reset();
		if (selectedentity->HasComponent<CameraComponent>()) {
			CameraComponent cc = selectedentity->GetComponent<CameraComponent>();
			s_CopiedEntity.cc = cc;
		}

		// copy spriterenderercomponent
		s_CopiedEntity.spc.reset();
		if (selectedentity->HasComponent<SpriteRendererComponent>()) {
			SpriteRendererComponent spc = selectedentity->GetComponent<SpriteRendererComponent>();
			s_CopiedEntity.spc = spc;
		}

		// copy Fontrenderercomponent
		s_CopiedEntity.fc.reset();
		if (selectedentity->HasComponent<FontRendererComponent>()) {
			FontRendererComponent fc = selectedentity->GetComponent<FontRendererComponent>();
			s_CopiedEntity.fc = fc;
		}

		// copy script component
		s_CopiedEntity.sc.reset();
		if (selectedentity->HasComponent<ScriptComponent>()) {
			ScriptComponent sc = selectedentity->GetComponent<ScriptComponent>();
			s_CopiedEntity.sc = sc;
		}

		// copy animationgroupcomponent
		s_CopiedEntity.agc.reset();
		if (selectedentity->HasComponent<AnimationGroupComponent>()) {
			AnimationGroupComponent agc = selectedentity->GetComponent<AnimationGroupComponent>();
			s_CopiedEntity.agc = agc;
		}

		// copy rigidbody2dComponent
		s_CopiedEntity.rbc.reset();
		if (selectedentity->HasComponent<RigidBody2DComponent>()) {
			RigidBody2DComponent rbc = selectedentity->GetComponent<RigidBody2DComponent>();
			s_CopiedEntity.rbc = rbc;
		}

		// copy boxcollider2dcomponenttt
		s_CopiedEntity.bcc.reset();
		if (selectedentity->HasComponent<BoxCollider2DComponent>()) {
			BoxCollider2DComponent bcc = selectedentity->GetComponent<BoxCollider2DComponent>();
			s_CopiedEntity.bcc = bcc;
		}

		// copy chainshapecollidercomponent
		s_CopiedEntity.csc.reset();
		if (selectedentity->HasComponent<ChainShapeColliderComponent>()) {
			ChainShapeColliderComponent csc = selectedentity->GetComponent<ChainShapeColliderComponent>();
			s_CopiedEntity.csc = csc;
		}

		// copy particlegeneratorcomponent
		s_CopiedEntity.pgc.reset();
		if (selectedentity->HasComponent<ParticleGeneratorComponent>()) {
			ParticleGeneratorComponent pgc = selectedentity->GetComponent<ParticleGeneratorComponent>();
			s_CopiedEntity.pgc = pgc;
		}

		// copy model3dcomponent
		s_CopiedEntity.m3c.reset();
		if (selectedentity->HasComponent<Model3DComponent>()) {
			Model3DComponent mc = selectedentity->GetComponent<Model3DComponent>();
			s_CopiedEntity.m3c = mc;

		}

		// copy lightcomponent
		s_CopiedEntity.lc.reset();
		if (selectedentity->HasComponent<LightComponent>()) {
			LightComponent lc = selectedentity->GetComponent<LightComponent>();
			s_CopiedEntity.lc = lc;
		}

		// copy physicscomponent
		s_CopiedEntity.pc.reset();
		if (selectedentity->HasComponent<PhysicsComponent>()) {
			PhysicsComponent pc = selectedentity->GetComponent<PhysicsComponent>();

			s_CopiedEntity.pc = pc;

		}


		// copy animation 3d component
		s_CopiedEntity.a3c.reset();
		if (selectedentity->HasComponent<Animation3DComponent>()) {
			Animation3DComponent a3c = selectedentity->GetComponent<Animation3DComponent>();
			s_CopiedEntity.a3c = a3c;
		}
	}
}

void EditorLayer::PasteEntity() {
	if (s_CopiedEntity.nc.has_value()) {
		std::shared_ptr<Entity> entity = selectedscene->CreateEntity(s_CopiedEntity.nc.value().name);
		// copy position
		TransformComponent tcopy = s_CopiedEntity.tc.value();
		TransformComponent& tc = entity->GetComponent<TransformComponent>();
		tc.position = tcopy.position;
		tc.rotation = tcopy.rotation;
		tc.scale = tcopy.scale;
		tc.SetRotationEuler(tc.rotation);


		// add camera component
		if (s_CopiedEntity.cc.has_value()) {
			CameraComponent cccopy = s_CopiedEntity.cc.value();
			CameraComponent& cc = entity->AddComponent<CameraComponent>(cccopy.camerao, cccopy.Primary);
		}


		// add spriterenderercomponent
		if (s_CopiedEntity.spc.has_value()) {
			SpriteRendererComponent spccopy = s_CopiedEntity.spc.value();
			SpriteRendererComponent& sp = entity->AddComponent<SpriteRendererComponent>(spccopy.m_subtexture, spccopy.color, spccopy.type);
		}

		// add FontRendererComponent
		if (s_CopiedEntity.fc.has_value()) {
			FontRendererComponent fccopy = s_CopiedEntity.fc.value();
			FontRendererComponent& fc = entity->AddComponent<FontRendererComponent>(fccopy.text, fccopy.pixelsize, fccopy.opacity);
		}


		// add ScriptComponent
		if (s_CopiedEntity.sc.has_value()) {
			ScriptComponent sccopy = s_CopiedEntity.sc.value();
			ScriptComponent& sc = entity->AddComponent<ScriptComponent>(sccopy.scriptname);
		}

		// add AnimationGroupComponent
		if (s_CopiedEntity.agc.has_value()) {
			AnimationGroupComponent agccopy = s_CopiedEntity.agc.value();
			AnimationGroupComponent& agc = entity->AddComponent<AnimationGroupComponent>();
			agc.animations = agccopy.animations;
			agc.animationpath = agccopy.animationpath;

		}

		// add rigidbody2dcomponent

		if (s_CopiedEntity.rbc.has_value()) {
			RigidBody2DComponent rbccopy = s_CopiedEntity.rbc.value();
			RigidBody2DComponent& rbc = entity->AddComponent<RigidBody2DComponent>();
		}

		// add boxcollider2dcomponent
		if (s_CopiedEntity.bcc.has_value()) {
			BoxCollider2DComponent bccopy = s_CopiedEntity.bcc.value();
			BoxCollider2DComponent& bc = entity->AddComponent<BoxCollider2DComponent>(bccopy.density, bccopy.friction, bccopy.width, bccopy.height, bccopy.objectid, bccopy.sameasscale, bccopy.isSensor);
		}

		// add chainshapecollider
		if (s_CopiedEntity.csc.has_value()) {
			ChainShapeColliderComponent csccopy = s_CopiedEntity.csc.value();
			ChainShapeColliderComponent& csc = entity->AddComponent<ChainShapeColliderComponent>();
			csc.vertices = csccopy.vertices;
			csc.objectid = csccopy.objectid;
		}

		// add particle generator component
		if (s_CopiedEntity.pgc.has_value()) {
			ParticleGeneratorComponent pgccopy = s_CopiedEntity.pgc.value();
			ParticleGeneratorComponent& pgc = entity->AddComponent<ParticleGeneratorComponent>();
			pgc.m_particleprops = pgccopy.m_particleprops;
			pgc.time = pgccopy.time;
			pgc.count = pgccopy.count;
			pgc.active = pgccopy.active;
		}

		// add model3dcomponent

		if (s_CopiedEntity.m3c.has_value()) {
			Model3DComponent m3c = s_CopiedEntity.m3c.value();
			Model3DComponent& m = entity->AddComponent<Model3DComponent>();
			m.filepath = m3c.filepath;
			m.mModel = ModelManager::GetModel(m.filepath);
		}

		// add lightcomponent
		if (s_CopiedEntity.lc.has_value()) {
			LightComponent lc = s_CopiedEntity.lc.value();
			LightComponent& l = entity->AddComponent<LightComponent>();
			l = lc;
			l.m_Texture = nullptr;
			l.Update();
		}

		// add physicscomponent
		if (s_CopiedEntity.pc.has_value()) {
			PhysicsComponent pc = s_CopiedEntity.pc.value();
			PhysicsComponent& p = entity->AddComponent<PhysicsComponent>();
			p = pc;
		}

		// add animation3dcomponent
		if (s_CopiedEntity.a3c.has_value()) {
			Animation3DComponent a3c = s_CopiedEntity.a3c.value();
			Animation3DComponent& a = entity->AddComponent<Animation3DComponent>();
			a = a3c;

			Model3DComponent mc = s_CopiedEntity.m3c.value();
			a.Reload(mc.filepath, mc.mModel.get());
		}
	}
}


EditorLayer::~EditorLayer() {

}
