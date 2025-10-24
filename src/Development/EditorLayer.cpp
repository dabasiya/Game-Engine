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


SubTexture EditorLayer::particlegeneratorsubtexture;
CopiedEntity EditorLayer::s_CopiedEntity;
CameraTransform EditorLayer::s_CameraTransform;



// Labels for Options
std::vector<std::string> Label_XYZ = { "X", "Y", "Z" };
std::vector<std::string> Label_RGB = { "R", "G", "B" };

std::vector<std::string> Label_LightTypes = { "Directional", "SpotLight", "PointLight" };

EditorLayer::EditorLayer() {
	ui::windowdatas.push_back(&windowmanagerdata);
	ui::windowdatas.push_back(&scenehierarchywindowdata);
	ui::windowdatas.push_back(&propertieswindowdata);
	ui::windowdatas.push_back(&subtexturewindowdata);
	ui::windowdatas.push_back(&animationcreatorwindowdata);
	ui::windowdatas.push_back(&texturewindowdata);
	ui::windowdatas.push_back(&entityregistrarwindowdata);
	ui::windowdatas.push_back(&skyboxwindowdata);

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


	EditorLayerIcons["ObjectGizmos"] = { 13, {0.0f, 1.0f}, {0.125f, 0.875f} };
	EditorLayerIcons["ColliderGizmos"] = { 13, {0.125f, 1.0f}, {0.25f, 0.875f} };
}


void EditorLayer::RenderEntityHierarchyTree(std::shared_ptr<Entity> entity, unsigned int offset) {

	auto& namec = entity->GetComponent<NameComponent>();
	auto& rc = entity->GetComponent<RelationshipComponent>();


	if (ui::DropDownButton(namec.name, rc.isUIOpen, offset)) {
		selectedentity->id = entity->id;
		selectedentity->m_scene = selectedscene;
		if (glfwGetKey(Window::ID, GLFW_KEY_C)) {
			auto ce = selectedentity->CreateChildEntity("Entity");
			Scene::s_msgbatch.Add(MessageType::info, "Created Child Entity.");
		}
		else if (glfwGetKey(Window::ID, GLFW_KEY_LEFT_ALT)) {
			selectedscene->DestroyEntity(selectedentity);
			Scene::s_msgbatch.Add(MessageType::info, "Entity Deleted.");
			selectedentity->id = (entt::entity)0;
			selectedentity->m_scene = nullptr;

			s_GizmoMode = ObjectMode;
		}
	}

	if (rc.isUIOpen) {
		for (auto e : rc.childEntities) {
			RenderEntityHierarchyTree(e, offset + 5);
		}
	}

}



//for vector 3 input box
void vector3f(const std::string& parentid, const std::vector<std::string>& labels, glm::vec3& values, float onclickvalue = 0.0f) {

	for (int i = 0; i < 3; i++) {
		ui::Label(labels[i]);

		std::string id = parentid + labels[i] + std::to_string(i);
		ui::FloatInputBox(id, values[i], onclickvalue);
	}
}

void vector4f(const std::string& parentid, const std::vector<std::string>& labels, glm::vec4& values, float onclickvalue = 0.0f) {
	for (int i = 0; i < 4; i++) {
		ui::Label(labels[i]);

		std::string id = parentid + labels[i] + std::to_string(i);
		ui::FloatInputBox(id, values[i], onclickvalue);
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

	ui::activeinputbox.id = "";
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
		//ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();

		float width = ImGui::GetIO().DisplaySize.x;
		float height = ImGui::GetIO().DisplaySize.y;
		ImGuizmo::SetRect(0.0f, 0.0f, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
		ImGuizmo::Enable(true);

		std::shared_ptr<Entity> cameraentity = selectedscene->GetPrimaryCameraEntity();
		auto& cameracp = cameraentity->GetComponent<CameraComponent>();
		auto& ctc = cameraentity->GetComponent<TransformComponent>();
		glm::mat4 cameraprojection;

		if (cameracp.cameratype == CameraType::Orthographic)
			cameraprojection = cameracp.camerao.viewprojection;
		else
			cameraprojection = cameracp.camerap.viewprojection;


		glm::mat4 cameraview = glm::mat4(1.0f);

		if (cameracp.cameratype == CameraType::Orthographic) {
			cameraview = glm::inverse(ctc.getmatrix());
		}
		else if (cameracp.cameratype == CameraType::Perspective) {
			cameraview = glm::lookAt(ctc.position, ctc.position + Scene::cameraorientation, glm::vec3(0.0f, 1.0f, 0.0f));
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

			transform = tc.worldtransform * tc.getmatrix();
		}
		else if (s_GizmoMode == ColliderMode) {
			op = ImGuizmo::OPERATION::SCALE;

			auto ttc = selectedentity->GetComponent<TransformComponent>();
			auto& pc = selectedentity->GetComponent<PhysicsComponent>();
			ttc.scale = pc.BoxHalfExtents * 2.0f;

			transform = ttc.worldtransform * ttc.getmatrix();
		}

		ImGuizmo::Manipulate(glm::value_ptr(cameraview), glm::value_ptr(cameraprojection), op, ImGuizmo::LOCAL, glm::value_ptr(transform));
		
		if (ImGuizmo::IsUsing()) {
			was_mousepressed = true;

			transform = glm::inverse(tc.worldtransform) * transform;

			glm::vec3 position, rotation, scale;
			Math::DecomposeTransform(transform, position, rotation, scale);

			float angle = tc.rotation.z * 180.0f / 3.14f;
			glm::vec3 angles = rotation * 180.0f / 3.14f;

			//tc.rotation.z += angle;
			//tc.rotation = angles;
			
			if (s_GizmoMode == ObjectMode) {
				tc.rotation = angles;
				tc.position = position;
				tc.scale = scale;
			}
			else if (s_GizmoMode == ColliderMode) {
				auto& pc = selectedentity->GetComponent<PhysicsComponent>();
				pc.BoxHalfExtents = scale / 2.0f;
			}
		}

		ImGui::End();
	}

}

void EditorLayer::Draw(float ts) {

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
	ui::windowmove();

	// calculate values used for ui element rendering
	ui::calculatevalues();

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

	float ui_ortho = Window::OrthographicSize / 2;
	
	glm::mat4 orthomatrix = glm::ortho(-Window::Ratio * ui_ortho, Window::Ratio * ui_ortho, -ui_ortho, ui_ortho);

	Renderer2D::UseShader(ShaderManager::Get("2d"));

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

		if (ui::Button("Null")) {}

		ui::Label("EditorCameraMoveSpeed");
		ui::FloatInputBox("editorcameramovespeed", EditorCameraSpeed, 0.0f);

		ui::popstyle();

		if (ui::Button("Reorder Rendering Sequence")) {
			selectedscene->reorder_rendering_sequence();
		}

		ui::CheckBox("Show Collider", EditorLayer::showcolliders);

		ui::Separator();

		if (ui::Button("Save Scene")) {
			auto app = Application::GetInstance();
			app.s_serializer.Serialize(app.s_serializer.filepath);
		}

		ui::Separator();

		std::string time = "Time : " + std::to_string(ts) + " ms";
		ui::Label(time);
		ui::End();

	}


	// Scene Hierarchy Window
	{
		ui::Begin("Scene Hierarchy", scenehierarchywindowdata);

		auto group = selectedscene->m_registry.group<NameComponent>();

		for (auto e : group) {
			auto namec = selectedscene->m_registry.get<NameComponent>(e);
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

				if (position)
					vector3f("entityposition", positionvaluesstring, transform.position);

				// for Rotation
				ui::StateButton("Rotation", rotation, false);
				ui::StateButton("(Vector3)", rotation, true);

				if (rotation)
					vector3f("entityrotation", positionvaluesstring, transform.rotation);

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

			// SpriteRendererComponent
			{
				if (selectedentity->HasComponent<SpriteRendererComponent>()) {
					auto& spc = selectedentity->GetComponent<SpriteRendererComponent>();

					ui::Separator();

					std::vector<std::string> colororsubtexture = { "Color", "SubTexture" };

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

						vector4f("subtexturecolor", colorvaluesstring, spc.color, 0.0f);

						ui::popstyle();

					}



					if (ui::Button("Delete SpriteRendererComponent")) {
						selectedentity->RemoveComponent<SpriteRendererComponent>();
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
						std::string id = "animationid" + std::to_string(i);
						std::string pathid = "animationpath" + std::to_string(i);
						ui::InputBox(id, agcp.animationpath[i].first);
						ui::InputBox(pathid, agcp.animationpath[i].second);
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

					ui::Label("VelocityVariation.x");
					ui::FloatInputBox("VelocityVariation.x", pg.m_particleprops.velocityvariation.x);
					ui::Label("VelocityVariation.y");
					ui::FloatInputBox("VelocityVariation.y", pg.m_particleprops.velocityvariation.y);

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
					ui::StateButton("ColorBegin", colorbegin, true);
					ui::StateButton("ColorEnd", colorbegin, false);

					vector4f("particlecolor", colorvaluesstring, colorbegin ? pg.m_particleprops.colorbegin : pg.m_particleprops.colorend, 0.0f);

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

					ui::Label("Shader");

					auto it = std::find(ShaderManager::s_shadernames.begin(), ShaderManager::s_shadernames.end(), mc.shadername);
					unsigned int index = 0;

					if (it != ShaderManager::s_shadernames.end()) {
						index = std::distance(ShaderManager::s_shadernames.begin(), it);
					}

					ui::OptionSelector(ShaderManager::s_shadernames, index);
					mc.shadername = ShaderManager::s_shadernames[index];

					ui::popstyle();

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
				if (selectedentity->HasComponent<LightComponent>()) {
					ui::Separator();

					auto& lc = selectedentity->GetComponent<LightComponent>();

					ui::pushstyle(uistyle::row_two_block);

					ui::Label("LightType");
					unsigned int lighttypes = lc.lighttype;
					ui::OptionSelector(Label_LightTypes, lighttypes);

					if (lighttypes != lc.lighttype) {
						lc.m_Texture.reset();
					}

					lc.lighttype = (LightType)lighttypes;

					ui::popstyle();

					ui::Label("Light Color");

					ui::pushstyle(uistyle::row_two_block);
					vector3f("LightComponentColor", Label_RGB, lc.color);

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

					static std::vector<std::string> shapetypes = {
						"Box",
						"Sphere",
						"Capsule",
						"Infinite Plane"
					};

					unsigned int index = (unsigned int)pc.ShapeType;
					ui::Label("Shape");
					ui::OptionSelector(shapetypes, index);
					pc.ShapeType = (PhysicsShapeType)index;


					ui::Label("Mass");
					ui::FloatInputBox("BulletMass", pc.Mass);

					if (pc.ShapeType == PhysicsShapeType::Box || pc.ShapeType == PhysicsShapeType::Capsule) {
						vector3f("Halfextent3", Label_XYZ, pc.BoxHalfExtents);
					}

					if (pc.ShapeType == PhysicsShapeType::Capsule || pc.ShapeType == PhysicsShapeType::Sphere) {
						ui::Label("Sphere Radius");
						ui::FloatInputBox("bulletsphereradius", pc.SphereRadius);
					}

					ui::Label("Restitution");
					ui::FloatInputBox("Restitution", pc.Restitution);

					ui::popstyle();

					if (ui::Button("Remove Physics Component")) {
						selectedentity->RemoveComponent<PhysicsComponent>();
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

				if (!selectedentity->HasComponent<FontRendererComponent>()) {
					if (ui::Button("Fontrenderer Component")) {
						selectedentity->AddComponent<FontRendererComponent>("", 14.0f);
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

				if (!selectedentity->HasComponent<ChainShapeColliderComponent>()) {
					if (ui::Button("ChainShapeCollider Component")) {
						selectedentity->AddComponent<ChainShapeColliderComponent>();
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
			ui::FloatInputBox(id, m_animation.frametimes[i]);
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
			ui::Label(number);
			std::string id = "texture" + number;
			ui::InputBox(id, Renderer2D::textures[i]->path);
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

			if (ui::Button(nc.name)) {
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

	// Other UI Stuff
	{
		Renderer2D::BeginScene(orthomatrix);
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
		Renderer2D::EndScene();

	}



	// Draw Console 
	{
		Renderer2D::BeginScene(orthomatrix);
		test_console.Draw();
		Renderer2D::EndScene();
	}


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
	}
}


EditorLayer::~EditorLayer() {

}