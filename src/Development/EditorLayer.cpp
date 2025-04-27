#include "Editorlayer.h"
#include <Scene/Components.h>
#include <Renderer2D.h>
#include <Application.h>
#include <Scene/AnimationUtil.h>
#include <WindowsInput.h>

#include <gladr.h>

Entity EditorLayer::selectedentity = { (entt::entity)0, nullptr };

#define selectedscene Application::s_serializer.m_scene

SubTexture* EditorLayer::selectedsubtexture = nullptr;

bool EditorLayer::was_mousepressed = false;

bool EditorLayer::record_chain_collider_vertices = false;

bool EditorLayer::mouse_on_window = false;

unsigned int EditorLayer::s_EntityEditingMode = 0;

bool EditorLayer::camera_locked = false;

bool EditorLayer::showcolliders = false;

SubTexture EditorLayer::particlegeneratorsubtexture;


EditorLayer::EditorLayer() {
	ui::windowdatas.push_back(&windowmanagerdata);
	ui::windowdatas.push_back(&scenehierarchywindowdata);
	ui::windowdatas.push_back(&propertieswindowdata);
	ui::windowdatas.push_back(&subtexturewindowdata);
	ui::windowdatas.push_back(&animationcreatorwindowdata);
	ui::windowdatas.push_back(&texturewindowdata);

	m_depthbuffer = new Texture(Window::Width, Window::Height, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, 0);
	m_colorbuffer = new Texture(Window::Width, Window::Height, GL_RGB8, GL_RGB, 0);

	m_entitybuffer.AddColorAttachment(0, *m_colorbuffer);
	m_entitybuffer.AddDepthAttachment(*m_depthbuffer);

	m_entitycolorshader = new Shader("res/shaders/entitycolor.vert", "res/shaders/entitycolor.frag");

	// particle generator subtexture
	particlegeneratorsubtexture.index = 15;
	particlegeneratorsubtexture.coords1 = { 0.0f, 1.0f };
	particlegeneratorsubtexture.coords2 = { 0.125f, 0.875f };
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



// for move entity by drag axis line on scene
bool EditorLayer::MoveEntity() {
	auto transform = selectedentity.GetComponent<TransformComponent>();
	float width = 0.02f;
	float height = 0.7f;

	bool moved = false;

	glm::mat4 cameramatrix = glm::mat4(1.0f);
	auto view = selectedscene->m_registry.view<TransformComponent, CameraComponent>();
	for (auto e : view) {
		auto [cc, transform1] = view.get<CameraComponent, TransformComponent>(e);
		if (cc.Primary) {
			cameramatrix = transform1.getmatrix();
			break;
		}
	}

	auto& transformref = selectedentity.GetComponent<TransformComponent>();

	glm::vec4 mousecoords = cameramatrix * glm::vec4(Window::Mousex, Window::Mousey, .0f, .0f);
	transform.position = glm::vec3(glm::inverse(cameramatrix) * glm::vec4(transform.position, 1.0f));

	if ((mousecoords.x >= transform.position.x - width * Window::OrthographicSize / 2 && mousecoords.x <= transform.position.x + width * Window::OrthographicSize / 2) && (mousecoords.y <= transform.position.y + height * Window::OrthographicSize / 2 && mousecoords.y >= transform.position.y)) {
		m_EntitySceneEvent.MouseDown = true;
		m_EntitySceneEvent.Axis = EntityGizmos::Y_AXIS;
	}

	if (m_EntitySceneEvent.MouseDown && m_EntitySceneEvent.Axis == EntityGizmos::Y_AXIS) {
		float diff = Window::Mousey - Window::Last_Mousey;
		if (s_EntityEditingMode == EntityEditingMode::TRANSLATE)
		{
			transformref.position += glm::vec3(0.0f, diff, 0.0f);
		}
		else if (s_EntityEditingMode == EntityEditingMode::SCALE)
		{
			transformref.scale.y += diff;
		}
		moved = true;
	}
	if ((mousecoords.x >= transform.position.x && mousecoords.x <= transform.position.x + height * Window::OrthographicSize / 2) && (mousecoords.y <= transform.position.y + width * Window::OrthographicSize / 2 && mousecoords.y >= transform.position.y - width * Window::OrthographicSize / 2)) {
		m_EntitySceneEvent.MouseDown = true;
		m_EntitySceneEvent.Axis = EntityGizmos::X_AXIS;
	}
	if (m_EntitySceneEvent.MouseDown && m_EntitySceneEvent.Axis == EntityGizmos::X_AXIS) {
		float diff = Window::Mousex - Window::Last_Mousex;
		if (s_EntityEditingMode == EntityEditingMode::TRANSLATE)
		{
			transformref.position += glm::vec3(diff, 0.0f, 0.0f);
		}
		else if (s_EntityEditingMode == EntityEditingMode::SCALE)
		{
			transformref.scale.x += diff;
		}
		else if (s_EntityEditingMode == EntityEditingMode::ROTATE) {
			transformref.rotation.z -= diff * 90.0f;
		}
		moved = true;
	}

	return moved;
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

	std::cout << n << std::endl;

	selectedentity = { (entt::entity)n, selectedscene };

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}


bool EditorLayer::OnEvent(Event& e) {

	if (ui::onevent(e))
		return true;

	if (test_console.OnEvent(e))
		return true;

	if (Input::KeyPressed(KEY_LEFT_CONTROL) && Input::KeyPressed(KEY_S)) {
		s_EntityEditingMode = EntityEditingMode::SCALE;
		camera_locked = true;
	}
	else if (Input::KeyPressed(KEY_LEFT_CONTROL) && Input::KeyPressed(KEY_T)) {
		s_EntityEditingMode = EntityEditingMode::TRANSLATE;
		camera_locked = true;
	}
	else if (Input::KeyPressed(KEY_LEFT_CONTROL) && Input::KeyPressed(KEY_R)) {
		s_EntityEditingMode = EntityEditingMode::ROTATE;
		camera_locked = true;
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
					cc.camera.orthographicsize += -(ms->yoffset - 1);
				else if (ms->yoffset > 0)
					cc.camera.orthographicsize -= (ms->yoffset + 1);

				if (cc.camera.orthographicsize < 0.5f)
					cc.camera.orthographicsize = 0.5f;
				Window::OrthographicSize = cc.camera.orthographicsize;
				cc.camera.Update();
				break;
			}
		}
	}


	return false;
}


void EditorLayer::Draw(float ts) {


	// if window resized then change framebuffer size which is used for entity selection

	if (m_colorbuffer->width != Window::Width || m_colorbuffer->height != Window::Height) {
		delete m_depthbuffer;
		delete m_colorbuffer;


		m_depthbuffer = new Texture(Window::Width, Window::Height, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, 0);
		m_colorbuffer = new Texture(Window::Width, Window::Height, GL_RGB8, GL_RGB, 0);

		m_entitybuffer.AddColorAttachment(0, *m_colorbuffer);
		m_entitybuffer.AddDepthAttachment(*m_depthbuffer);
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


	// for selecting entity and moving entity
	bool entity_moved = false;
	if (!test_console.visible) {

		// for entity translation, rotation, scale
		if (selectedentity.m_scene != nullptr) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1)) {
				entity_moved = MoveEntity();
			}

			else if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE) {
				m_EntitySceneEvent.MouseDown = false;
			}
		}

		// for selecting entity
		if (!entity_moved && !was_mousepressed && !record_chain_collider_vertices && !mouse_on_window) {
			if (glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1)) {
				SelectEntity();
			}
		}

		// for record vertices of chain shape collider
		else if (record_chain_collider_vertices && selectedentity.m_scene != nullptr && selectedentity.HasComponent<ChainShapeColliderComponent>() && !was_mousepressed && glfwGetMouseButton(Window::ID, GLFW_MOUSE_BUTTON_1)) {
			glm::vec4 mousepos = glm::vec4(Window::Mousex, Window::Mousey, 0.0f, 0.0f);

			was_mousepressed = true;

			glm::mat4 cameratransform = glm::mat4(1.0f);
			auto view = selectedscene->m_registry.view<TransformComponent, CameraComponent>();
			for (auto e : view) {
				auto [cc, transform] = view.get<CameraComponent, TransformComponent>(e);
				if (cc.Primary) {
					cameratransform = transform.getmatrix();
					break;
				}
			}

			mousepos = cameratransform * mousepos;

			auto transform = selectedentity.GetComponent<TransformComponent>();

			transform.position = glm::vec3(glm::inverse(cameratransform) * glm::vec4(transform.position, 1.0f));

			glm::vec2 mousepos2 = glm::vec2(mousepos.x - transform.position.x, mousepos.y - transform.position.y);

			auto& cs = selectedentity.GetComponent<ChainShapeColliderComponent>();
			cs.vertices.push_back(b2Vec2(mousepos2.x, mousepos2.y));

		}
	}

	windowmanagerdata.visible = true;


	// for rendering gizmos

	float ui_ortho = Window::OrthographicSize / 2;
	glm::mat4 orthomatrix = glm::ortho(-Window::Ratio * ui_ortho, Window::Ratio * ui_ortho, -ui_ortho, ui_ortho, -1.0f, 1.0f);
	Renderer2D::BeginScene(orthomatrix);

	if (selectedentity.m_scene != nullptr) {
		auto transform = selectedentity.GetComponent<TransformComponent>();
		float width = 0.02f;
		float height = 0.7f;

		glm::mat4 cameramatrix = glm::mat4(1.0f);
		auto view = selectedscene->m_registry.view<TransformComponent, CameraComponent>();
		for (auto e : view) {
			auto [cc, transform1] = view.get<CameraComponent, TransformComponent>(e);
			if (cc.Primary) {
				cameramatrix = transform1.getmatrix();
				break;
			}
		}
		transform.position = glm::vec3(glm::inverse(cameramatrix) * glm::vec4(transform.position, 1.0f));

		glm::vec4 mousecoords = glm::vec4(Window::Mousex, Window::Mousey, .0f, .0f);

		glm::vec4 y_color = { 1.0f, .0f, .0f, 1.0f };
		glm::vec4 x_color = { .0f, 1.0f, .0f, 1.0f };

		if ((mousecoords.x >= transform.position.x - width * Window::OrthographicSize / 2 && mousecoords.x <= transform.position.x + width * Window::OrthographicSize / 2) && (mousecoords.y <= transform.position.y + height * Window::OrthographicSize / 2 && mousecoords.y >= transform.position.y)) {
			y_color = glm::vec4(1.0f);
		}

		if ((mousecoords.x >= transform.position.x && mousecoords.x <= transform.position.x + height * Window::OrthographicSize / 2) && (mousecoords.y <= transform.position.y + width * Window::OrthographicSize / 2 && mousecoords.y >= transform.position.y - width * Window::OrthographicSize / 2)) {
			x_color = glm::vec4(1.0f);
		}

		if (m_EntitySceneEvent.MouseDown && m_EntitySceneEvent.Axis == EntityGizmos::X_AXIS) {
			x_color = glm::vec4(1.0f);
		}
		else if (m_EntitySceneEvent.MouseDown && m_EntitySceneEvent.Axis == EntityGizmos::Y_AXIS) {
			y_color = glm::vec4(1.0f);
		}

		// this render two arrows quad if editing mode is translate or scale
		// if editingmode is rotation then it renders only 1 quad which is on axis
		if (s_EntityEditingMode != EntityEditingMode::ROTATE)
			Renderer2D::DrawQuad(glm::vec3(transform.position.x, transform.position.y, 0.02f), { width * Window::OrthographicSize / 2, height * Window::OrthographicSize / 2 }, y_color, { 0.0f, 0.5f });
		Renderer2D::DrawQuad(glm::vec3(transform.position.x, transform.position.y, 0.02f), { height * Window::OrthographicSize / 2, width * Window::OrthographicSize / 2 }, x_color, { 0.5f, 0.0f });

		if (s_EntityEditingMode == EntityEditingMode::TRANSLATE) {
			Renderer2D::DrawRotatedTriangle(glm::vec3(transform.position.x, transform.position.y, 0.02f) + glm::vec3(height * Window::OrthographicSize / 2, 0.0f, 0.0f), { width * Window::OrthographicSize, width * Window::OrthographicSize }, -90.0f, x_color, { 0.5f, 0.0f });
			Renderer2D::DrawRotatedTriangle(glm::vec3(transform.position.x, transform.position.y, 0.02f) + glm::vec3(0.0f, height * Window::OrthographicSize / 2, 0.0f), { width * Window::OrthographicSize, width * Window::OrthographicSize }, 0.0f, y_color, { 0.0f, 0.5f });
		}
		else if (s_EntityEditingMode == EntityEditingMode::SCALE) {
			Renderer2D::DrawQuad(glm::vec3(transform.position.x, transform.position.y, 0.02f) + glm::vec3(height * Window::OrthographicSize / 2, 0.0f, 0.0f), { width * Window::OrthographicSize, width * Window::OrthographicSize }, x_color);
			Renderer2D::DrawQuad(glm::vec3(transform.position.x, transform.position.y, 0.02f) + glm::vec3(0.0f, height * Window::OrthographicSize / 2, 0.0f), { width * Window::OrthographicSize, width * Window::OrthographicSize }, y_color);
		}
	}

	Renderer2D::EndScene();

	// window manager
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

	ui::popstyle();

	if (ui::Button("Reorder Rendering Sequence")) {
		selectedscene->reorder_rendering_sequence();
	}

	ui::CheckBox("Show Collider", EditorLayer::showcolliders);

	ui::Separator();

	if (ui::Button("Save Scene")) {
		Application::s_serializer.Serialize(Application::s_serializer.filepath);
	}

	ui::Separator();

	std::string time = "Time : " + std::to_string(ts) + " ms";
	ui::Label(time);
	ui::End();



	// scene hierarchy window
	ui::Begin("Scene Hierarchy", scenehierarchywindowdata);

	ui::pushstyle(uistyle::row_two_block);

	auto group = selectedscene->m_registry.group<NameComponent>();

	for (auto e : group) {
		auto namec = selectedscene->m_registry.get<NameComponent>(e);
		if (ui::Button(namec.name)) {
			selectedentity = { e, selectedscene };
			if (glfwGetKey(Window::ID, GLFW_KEY_LEFT_ALT)) {
				selectedscene->DestroyEntity(selectedentity);
				selectedentity = { (entt::entity)0, nullptr };
			}
		}
	}
	ui::popstyle();
	ui::Separator();
	if (ui::Button("Add Entity")) {
		selectedscene->CreateEntity("Entity");
	}
	ui::End();


	if (selectedentity.m_scene != nullptr) {
		ui::Begin("Properties", propertieswindowdata);
		ui::pushstyle(uistyle::row_two_block);

		ui::Label("Name");
		auto& namec = selectedentity.GetComponent<NameComponent>();
		ui::InputBox("entityname", namec.name);

		ui::Separator();

		auto& transform = selectedentity.GetComponent<TransformComponent>();


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

		// fontrenderercomponent

		if (selectedentity.HasComponent<FontRendererComponent>()) {
			auto& frc = selectedentity.GetComponent<FontRendererComponent>();

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
				selectedentity.RemoveComponent<FontRendererComponent>();
			}
		}

		// for sprite renderercomponent
		if (selectedentity.HasComponent<SpriteRendererComponent>()) {
			auto& spc = selectedentity.GetComponent<SpriteRendererComponent>();

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
				selectedentity.RemoveComponent<SpriteRendererComponent>();
			}
		}

		// for show cameracomponent detail
		if (selectedentity.HasComponent<CameraComponent>()) {
			ui::pushstyle(uistyle::row_two_block);
			ui::Separator();

			ui::StateButton("Camera", cameraprop, false);
			ui::StateButton("Show", cameraprop, true);

			if (cameraprop) {
				auto& cc = selectedentity.GetComponent<CameraComponent>();
				ui::Label("Near");
				ui::FloatInputBox("camera_near", cc.camera.near);

				ui::Label("Far");
				ui::FloatInputBox("camera_far", cc.camera.far);

				ui::Label("Min X");
				ui::FloatInputBox("cameraminx", cc.x_min);

				ui::Label("Max X");
				ui::FloatInputBox("cameramaxx", cc.x_max);

				ui::Label("Min Y");
				ui::FloatInputBox("cameraminy", cc.y_min);

				ui::Label("Max Y");
				ui::FloatInputBox("cameramaxy", cc.y_max);


				ui::Label("Size");
				float temp = cc.camera.orthographicsize;
				ui::FloatInputBox("camera_size", cc.camera.orthographicsize);

				if (temp != cc.camera.orthographicsize)
					cc.camera.Update();


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
				selectedentity.RemoveComponent<CameraComponent>();
			}
		}



		// for showing scriptcomponent detail
		if (selectedentity.HasComponent<ScriptComponent>()) {
			auto& scriptc = selectedentity.GetComponent<ScriptComponent>();
			ui::pushstyle(uistyle::row_two_block);
			ui::Separator();
			ui::Label("Script");
			ui::InputBox("script_name", scriptc.scriptname);
			ui::popstyle();

			if (ui::Button("Load Script")) {
				scriptc.destroy();
			}

			if (ui::Button("Delete ScriptComponent")) {
				selectedentity.RemoveComponent<ScriptComponent>();
			}
		}


		// for showing rigidbody component
		if (selectedentity.HasComponent<RigidBody2DComponent>()) {
			auto& rb2d = selectedentity.GetComponent<RigidBody2DComponent>();
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
				selectedentity.RemoveComponent<RigidBody2DComponent>();
			}
		}

		// for box collider component
		if (selectedentity.HasComponent<BoxCollider2DComponent>()) {
			auto& bc2d = selectedentity.GetComponent<BoxCollider2DComponent>();
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
			ui::popstyle();

			if (ui::Button("Delete BoxCollider2DComponent")) {
				selectedentity.RemoveComponent<BoxCollider2DComponent>();
			}
		}

		// for chainShape collider component
		if (selectedentity.HasComponent<ChainShapeColliderComponent>()) {
			auto& cs = selectedentity.GetComponent<ChainShapeColliderComponent>();
			ui::Separator();

			ui::pushstyle(uistyle::row_two_block);

			ui::Label("ObjectID");
			ui::UIntInputBox("chainshapecolliderid", cs.objectid, 0);

			ui::popstyle();

			if (!record_chain_collider_vertices) {
				if (ui::Button("Start Record Vertices")) {
					record_chain_collider_vertices = !record_chain_collider_vertices;
					cs.vertices.clear();
				}
			}
			else {
				if (ui::Button("Stop Record Vertices")) {
					record_chain_collider_vertices = !record_chain_collider_vertices;
					cs.vertices.pop_back();
				}
			}

			if (ui::Button("Delete ChainShapeColliderComponent")) {
				selectedentity.RemoveComponent<ChainShapeColliderComponent>();
			}
		}

		// for animationgroup component
		if (selectedentity.HasComponent<AnimationGroupComponent>()) {
			auto& agcp = selectedentity.GetComponent<AnimationGroupComponent>();

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
				selectedentity.RemoveComponent<AnimationGroupComponent>();
			}
		}

		if (selectedentity.HasComponent<ParticleGeneratorComponent>()) {
			auto& pg = selectedentity.GetComponent<ParticleGeneratorComponent>();
			auto& tc = selectedentity.GetComponent<TransformComponent>();


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
				selectedentity.RemoveComponent<ParticleGeneratorComponent>();
			}
		}

		ui::Separator();

		// for adding other components

		if (!selectedentity.HasComponent<CameraComponent>()) {
			if (ui::Button("Camera Component")) {
				selectedentity.AddComponent<CameraComponent>();
			}
		}

		if (!selectedentity.HasComponent<SpriteRendererComponent>()) {
			if (ui::Button("Sprite Component")) {
				selectedentity.AddComponent<SpriteRendererComponent>();
			}
		}

		if (!selectedentity.HasComponent<FontRendererComponent>()) {
			if (ui::Button("Fontrenderer Component")) {
				selectedentity.AddComponent<FontRendererComponent>("", 14.0f);
			}
		}

		if (!selectedentity.HasComponent<ScriptComponent>()) {
			if (ui::Button("Script Component")) {
				selectedentity.AddComponent<ScriptComponent>("");
			}
		}

		if (!selectedentity.HasComponent<RigidBody2DComponent>()) {
			if (ui::Button("RigidBody2D Component")) {
				selectedentity.AddComponent<RigidBody2DComponent>();
			}
		}

		if (!selectedentity.HasComponent<BoxCollider2DComponent>()) {
			if (ui::Button("BoxCollider2D Component")) {
				auto transform = selectedentity.GetComponent<TransformComponent>();
				auto& bc = selectedentity.AddComponent<BoxCollider2DComponent>();
				bc.width = transform.scale.x;
				bc.height = transform.scale.y;
			}
		}

		if (!selectedentity.HasComponent<AnimationGroupComponent>()) {
			if (ui::Button("AnimationGroup Component")) {
				selectedentity.AddComponent<AnimationGroupComponent>();
			}
		}

		if (!selectedentity.HasComponent<ChainShapeColliderComponent>()) {
			if (ui::Button("ChainShapeCollider Component")) {
				selectedentity.AddComponent<ChainShapeColliderComponent>();
			}
		}

		if (!selectedentity.HasComponent<ParticleGeneratorComponent>()) {
			if (ui::Button("Particle Generator Component")) {
				selectedentity.AddComponent<ParticleGeneratorComponent>();
			}
		}

		ui::End();
	}

	// subtexture set window 
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

	// animation creator window
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

	// texture window

	ui::Begin("Textures", texturewindowdata);
	ui::pushstyle(uistyle::row_two_block);
	for (uint32_t i = 2; i < 16; i++) {
		if (!Renderer2D::textures[i]) {
			Renderer2D::textures[i] = new Texture(1, 1, GL_RGBA8, GL_RGBA, 0);
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
				delete Renderer2D::textures[i];
				Renderer2D::textures[i] = new Texture(path.c_str());
			}
		}
	}
	ui::End();

	Renderer2D::BeginScene(orthomatrix);
	test_console.Draw();
	Renderer2D::EndScene();

	m_entitybuffer.Bind();

	Renderer2D::UseShader(m_entitycolorshader);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 camproj = glm::mat4(1.0f);
	auto view = selectedscene->m_registry.view<CameraComponent, TransformComponent>();
	for (auto e : view) {
		auto& cc = view.get<CameraComponent>(e);
		auto& tc = view.get<TransformComponent>(e);
		if (cc.Primary) {
			cc.camera.Update();
			camproj = cc.camera.viewprojection * glm::inverse(tc.getmatrix());
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
