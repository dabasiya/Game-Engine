#include "Serializer.h"
#include "Entity.h"
#include "Components.h"
#include <Renderer2D.h>
#include <fstream>

#include <syaml.h>

Serializer::Serializer(Scene* a_scene)
	: m_scene(a_scene)
{

}


static void serializeentity(YAML::Emitter& out, Entity e) {
	out << YAML::BeginMap;
	out << YAML::Key << "Entity" << YAML::Value << (unsigned int)e.id;


	// NameComponent
	out << YAML::Key << "NameComponent" << YAML::BeginMap;

	auto& nc = e.GetComponent<NameComponent>();
	out << YAML::Key << "name" << YAML::Value << nc.name;
	out << YAML::Key << "active" << YAML::Value << nc.active;

	out << YAML::EndMap;

	// transform component
	out << YAML::Key << "TransformComponent" << YAML::BeginMap;

	auto& transform = e.GetComponent<TransformComponent>();
	out << YAML::Key << "position" << YAML::Value << transform.position;
	out << YAML::Key << "rotation" << YAML::Value << transform.rotation;
	out << YAML::Key << "scale" << YAML::Value << transform.scale;

	out << YAML::EndMap;

	// SpriteRendererComponent
	if (e.HasComponent<SpriteRendererComponent>()) {
		out << YAML::Key << "SpriteRendererComponent" << YAML::BeginMap;

		auto& sc = e.GetComponent<SpriteRendererComponent>();
		out << YAML::Key << "subtexture" << sc.m_subtexture;
		out << YAML::Key << "type" << YAML::Value << sc.type;
		out << YAML::Key << "color" << YAML::Value << sc.color;
		out << YAML::Key << "transparent" << YAML::Value << sc.transparent;

		out << YAML::EndMap;
	}

	// CameraComponent
	if (e.HasComponent<CameraComponent>()) {
		auto& cc = e.GetComponent<CameraComponent>();

		out << YAML::Key << "CameraComponent" << YAML::BeginMap;
                out << YAML::Key << "min_x" << YAML::Value << cc.x_min;
                out << YAML::Key << "max_x" << YAML::Value << cc.x_max;
                out << YAML::Key << "min_y" << YAML::Value << cc.y_min;
                out << YAML::Key << "max_y" << YAML::Value << cc.y_max;
		out << YAML::Key << "camera" << YAML::BeginMap;
		out << YAML::Key << "near" << YAML::Value << cc.camera.near;
		out << YAML::Key << "far" << YAML::Value << cc.camera.far;
		out << YAML::Key << "size" << YAML::Value << cc.camera.orthographicsize;
		out << YAML::EndMap;

		out << YAML::Key << "primary" << YAML::Value << cc.Primary;

		out << YAML::EndMap;
	}

	// FontRendererComponent
	if (e.HasComponent<FontRendererComponent>()) {
		auto& frc = e.GetComponent<FontRendererComponent>();

		out << YAML::Key << "FontRendererComponent" << YAML::BeginMap;
		out << YAML::Key << "text" << YAML::Value << frc.text;
		out << YAML::Key << "opacity" << YAML::Value << frc.opacity;
		out << YAML::Key << "pixelsize" << YAML::Value << frc.pixelsize;
		out << YAML::EndMap;
	}

	// ScriptComponent
	if (e.HasComponent<ScriptComponent>()) {
		out << YAML::Key << "ScriptComponent" << YAML::BeginMap;

		auto& sc = e.GetComponent<ScriptComponent>();
		out << YAML::Key << "name" << YAML::Value << sc.scriptname;
		out << YAML::EndMap;
	}

	// RigidBody2DComponent
	if (e.HasComponent<RigidBody2DComponent>()) {
		out << YAML::Key << "RigidBody2DComponent" << YAML::BeginMap;
		auto& rb2d = e.GetComponent<RigidBody2DComponent>();
		out << YAML::Key << "fixedrotation" << YAML::Value << rb2d.fixedrotation;
		out << YAML::Key << "type" << YAML::Value << rb2d.type;
		out << YAML::Key << "id" << YAML::Value << rb2d.id;
		out << YAML::EndMap;
	}

	// BoxCollider2DComponent
	if (e.HasComponent<BoxCollider2DComponent>()) {
		out << YAML::Key << "BoxCollider2DComponent" << YAML::BeginMap;
		auto& b2d = e.GetComponent<BoxCollider2DComponent>();
		out << YAML::Key << "density" << YAML::Value << b2d.density;
		out << YAML::Key << "friction" << YAML::Value << b2d.friction;
		out << YAML::Key << "objectid" << YAML::Value << b2d.objectid;
		out << YAML::Key << "width" << YAML::Value << b2d.width;
		out << YAML::Key << "height" << YAML::Value << b2d.height;
		out << YAML::Key << "isSensor" << YAML::Value << b2d.isSensor;
		out << YAML::EndMap;
	}

	// ChainShapeColliderComponent
	if (e.HasComponent<ChainShapeColliderComponent>()) {
		out << YAML::Key << "ChainShapeColliderComponent" << YAML::BeginMap;
		auto& csc = e.GetComponent<ChainShapeColliderComponent>();

		int size = csc.vertices.size();

		for (unsigned int i = 0; i < size; i++) {
			std::string name = std::to_string(i);
			out << YAML::Key << i << YAML::BeginMap;
			out << YAML::Key << "x" << YAML::Value << csc.vertices[i].x;
			out << YAML::Key << "y" << YAML::Value << csc.vertices[i].y;
			out << YAML::EndMap;
		}
		out << YAML::Key << "objectid" << YAML::Value << csc.objectid;
		out << YAML::EndMap;
	}

	// AnimationGroupComponent
	if (e.HasComponent<AnimationGroupComponent>()) {
		out << YAML::Key << "AnimationGroupComponent" << YAML::BeginMap;
		auto& agcp = e.GetComponent<AnimationGroupComponent>();
		for (unsigned int i = 0; i < agcp.animationpath.size(); i++) {
			out << YAML::Key << i << YAML::BeginMap;
			out << YAML::Key << "key" << YAML::Value << agcp.animationpath[i].first;
			out << YAML::Key << "path" << YAML::Value << agcp.animationpath[i].second;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;
	}

	// particlegeneratorcomponent
	if (e.HasComponent<ParticleGeneratorComponent>()) {
		out << YAML::Key << "ParticleGeneratorComponent" << YAML::BeginMap;
		auto& pg = e.GetComponent<ParticleGeneratorComponent>();
		out << YAML::Key << "subtexture" << YAML::Value << pg.m_particleprops.m_subtexture;
		out << YAML::Key << "count" << YAML::Value << pg.count;
		out << YAML::Key << "velocity" << YAML::Value << pg.m_particleprops.velocity;
		out << YAML::Key << "velocityvariation" << YAML::Value << pg.m_particleprops.velocityvariation;
		out << YAML::Key << "generate_particle_from_area" << YAML::Value << pg.m_particleprops.generate_from_area;
                out << YAML::Key << "active" << YAML::Value << pg.active;
		out << YAML::Key << "beginsize" << YAML::Value << pg.m_particleprops.sizebegin;
		out << YAML::Key << "endsize" << YAML::Value << pg.m_particleprops.sizeend;
		out << YAML::Key << "lifetime" << YAML::Value << pg.m_particleprops.lifetime;
		out << YAML::Key << "sizevariation" << YAML::Value << pg.m_particleprops.sizevariation;
		out << YAML::Key << "colorbegin" << YAML::Value << pg.m_particleprops.colorbegin;
		out << YAML::Key << "colorend" << YAML::Value << pg.m_particleprops.colorend;
		out << YAML::EndMap;
	}

	out << YAML::EndMap;
}

void Serializer::Serialize(const std::string& path) {
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Scene" << YAML::Value << "Undefined";

	// write texture data
	for (unsigned int i = 2; i < 16; i++) {
		if (Renderer2D::textures[i] && Renderer2D::textures[i]->path != "") {
			std::string textureid = "texture" + std::to_string(i);
			out << YAML::Key << textureid << YAML::BeginMap;
			out << YAML::Key << "path" << YAML::Value << Renderer2D::textures[i]->path;
			out << YAML::EndMap;
		}
	}


	out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;


	auto view = m_scene->m_registry.view<TransformComponent>();
	for (auto entityid : view) {
		Entity e = { entityid, m_scene };

		serializeentity(out, e);
	}

	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::ofstream os(path);
	os << out.c_str();
}


void Serializer::Deserialize(const std::string& path) {
	filepath = path;
	std::ifstream ifl(path);
	std::stringstream strstream;
	strstream << ifl.rdbuf();

	YAML::Node data = YAML::Load(strstream.str());
	if (!data["Scene"])
		return;


	/*
// delete texture if exist
for (unsigned int i = 2; i < 16; i++) {
	if (Renderer2D::textures[i] != nullptr) {
		delete Renderer2D::textures[i];
		Renderer2D::textures[i] = nullptr;
		std::cout << "Texture " << i << " Deleted!" << std::endl;
	}
}
	*/

	// load textures
	for (unsigned int i = 2; i < 16; i++) {
		std::string textureid = "texture" + std::to_string(i);
		if (data[textureid]) {
			std::string path = data[textureid]["path"].as<std::string>();
			if (!Renderer2D::textures[i]) {
				Texture* t = new Texture(path.c_str());
				Renderer2D::SetTexture(t, i);
			}
			else if (Renderer2D::textures[i]->path != path) {
				delete Renderer2D::textures[i];
				Texture* t = new Texture(path.c_str());
				Renderer2D::SetTexture(t, i);
			}
		}
	}

	auto entities = data["Entities"];
	if (entities) {
		for (auto e : entities) {

			uint32_t id = e["Entity"].as<uint32_t>();

			// name component
			auto nc = e["NameComponent"];
			std::string name = nc["name"].as<std::string>();


			Entity deserializedentity = m_scene->CreateEntity(name);

			auto& nc2 = deserializedentity.GetComponent<NameComponent>();
			nc2.active = nc["active"].as<bool>();

			// TransformComponent
			auto tc = e["TransformComponent"];
			auto& transform = deserializedentity.GetComponent<TransformComponent>();
			transform.position = tc["position"].as<glm::vec3>();
			transform.rotation = tc["rotation"].as<glm::vec3>();
			transform.scale = tc["scale"].as<glm::vec3>();

			// SpriteRendererComponent
			auto sc = e["SpriteRendererComponent"];
			if (sc) {

				auto& sprite = deserializedentity.AddComponent<SpriteRendererComponent>();

				sprite.m_subtexture.coords1 = sc["subtexture"]["coords1"].as<glm::vec2>();
				sprite.m_subtexture.coords2 = sc["subtexture"]["coords2"].as<glm::vec2>();
				sprite.m_subtexture.index = sc["subtexture"]["index"].as<unsigned int>();
				sprite.transparent = sc["transparent"].as<bool>();
				sprite.type = sc["type"].as<bool>();

				sprite.color = sc["color"].as<glm::vec4>();
			}

			// CameraComponent
			auto cc = e["CameraComponent"];
			if (cc) {

				auto& camerac = deserializedentity.AddComponent<CameraComponent>();

				camerac.camera.near = cc["camera"]["near"].as<float>();
				camerac.camera.far = cc["camera"]["far"].as<float>();
				camerac.camera.orthographicsize = cc["camera"]["size"].as<float>();

				camerac.camera.Update();

                                
                                
                                camerac.x_min = cc["min_x"].as<float>();
                                camerac.x_max = cc["max_x"].as<float>();
                                camerac.y_min = cc["min_y"].as<float>();
                                camerac.y_max = cc["max_y"].as<float>();
                                
				camerac.Primary = cc["primary"].as<bool>();
				if (camerac.Primary)
					Window::OrthographicSize = camerac.camera.orthographicsize;
			}

			// FontRendererComponent
			auto frc = e["FontRendererComponent"];
			if (frc) {
				auto& fontrendererc = deserializedentity.AddComponent<FontRendererComponent>("", 0.0f);

				fontrendererc.text = frc["text"].as<std::string>();
				fontrendererc.opacity = frc["opacity"].as<float>();
				fontrendererc.pixelsize = frc["pixelsize"].as<float>();
			}

			// ScriptComponent
			auto scc = e["ScriptComponent"];
			if (scc) {
				std::string name = scc["name"].as<std::string>();
				auto& scriptc = deserializedentity.AddComponent<ScriptComponent>(name);
			}

			// RigidBody2DComponent
			auto rb2d = e["RigidBody2DComponent"];
			if (rb2d) {
				auto& rg2d = deserializedentity.AddComponent<RigidBody2DComponent>();
				rg2d.fixedrotation = rb2d["fixedrotation"].as<bool>();
				int type = rb2d["type"].as<int>();
				rg2d.type = (b2BodyType)type;
				rg2d.id = rb2d["id"].as<unsigned int>();
			}

			// BoxCollider2DComponent
			auto bx2d = e["BoxCollider2DComponent"];
			if (bx2d) {
				auto& b2d = deserializedentity.AddComponent<BoxCollider2DComponent>();
				b2d.density = bx2d["density"].as<float>();
				b2d.friction = bx2d["friction"].as<float>();
				b2d.width = bx2d["width"].as<float>();
				b2d.height = bx2d["height"].as<float>();
				b2d.objectid = bx2d["objectid"].as<unsigned int>();
				b2d.isSensor = bx2d["isSensor"].as<bool>();
			}

			// ChainShapeColliderComponent
			auto cs = e["ChainShapeColliderComponent"];
			if (cs) {
				auto& csc = deserializedentity.AddComponent<ChainShapeColliderComponent>();
				unsigned int index = 0;
				while (true) {
					std::string si = std::to_string(index);
					if (cs[si]) {
						float x = cs[si]["x"].as<float>();
						float y = cs[si]["y"].as<float>();
						csc.vertices.push_back(b2Vec2(x, y));
						index++;
					}
					else
						break;
				}
				csc.objectid = cs["objectid"].as<unsigned int>();
			}

			// AnimationGroupComponent
			auto agcp = e["AnimationGroupComponent"];
			if (agcp) {
				auto& agc = deserializedentity.AddComponent<AnimationGroupComponent>();
				unsigned int index = 0;
				while (true) {
					std::string strindex = std::to_string(index);
					if (agcp[strindex]) {
						agc.addanimationpath(agcp[strindex]["key"].as<std::string>(), agcp[strindex]["path"].as<std::string>());
					}
					else
						break;
					index++;
				}
				agc.loadanimations();
			}

			// particlegeneratorcomponent
			auto pg = e["ParticleGeneratorComponent"];
			if (pg) {
				auto& pgc = deserializedentity.AddComponent<ParticleGeneratorComponent>();
				pgc.count = pg["count"].as<unsigned int>();
				pgc.m_particleprops.velocity = pg["velocity"].as<glm::vec2>();
				pgc.m_particleprops.velocityvariation = pg["velocityvariation"].as<glm::vec2>();
				pgc.m_particleprops.sizebegin = pg["beginsize"].as<float>();
				pgc.m_particleprops.sizeend = pg["endsize"].as<float>();
				pgc.m_particleprops.lifetime = pg["lifetime"].as<float>();
				pgc.m_particleprops.sizevariation = pg["sizevariation"].as<float>();
				pgc.m_particleprops.generate_from_area = pg["generate_particle_from_area"].as<bool>();
                                pgc.active = pg["active"].as<bool>();
				pgc.m_particleprops.m_subtexture.coords1 = pg["subtexture"]["coords1"].as<glm::vec2>();
				pgc.m_particleprops.m_subtexture.coords2 = pg["subtexture"]["coords2"].as<glm::vec2>();
				pgc.m_particleprops.m_subtexture.index = pg["subtexture"]["index"].as<unsigned int>();


				// this scale and rotation values set by entity transform component
				auto& tc = deserializedentity.GetComponent<TransformComponent>();
				pgc.m_particleprops.scale = { tc.scale.x, tc.scale.y };
				pgc.m_particleprops.z_rotation = tc.rotation.z;

				pgc.m_particleprops.colorbegin = pg["colorbegin"].as<glm::vec4>();
				pgc.m_particleprops.colorend = pg["colorend"].as<glm::vec4>();
			}

		}
	}

	m_scene->reorder_rendering_sequence();
}
