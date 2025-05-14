#pragma once

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>
#include "SubTexture.h"


namespace YAML {

	template<>
	struct convert<glm::vec2> {
		static Node encode(const glm::vec2& value) {
			Node node;
			node.push_back(value.x);
			node.push_back(value.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& value) {
			if (!node.IsSequence() || node.size() != 2)
				return false;

			value.x = node[0].as<float>();
			value.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3> {
		static Node encode(const glm::vec3& value) {
			Node node;

			node.push_back(value.x);
			node.push_back(value.y);
			node.push_back(value.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& value) {
			if (!node.IsSequence() || node.size() != 3)
				return false;

			value.x = node[0].as<float>();
			value.y = node[1].as<float>();
			value.z = node[2].as<float>();
			return true;
		}

	};

	template<>
	struct convert<glm::vec4> {
		static Node encode(const glm::vec4& value) {
			Node node;
			node.push_back(value.x);
			node.push_back(value.y);
			node.push_back(value.z);
			node.push_back(value.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& value) {
			if (!node.IsSequence() || node.size() != 4)
				return false;

			value.x = node[0].as<float>();
			value.y = node[1].as<float>();
			value.z = node[2].as<float>();
			value.w = node[3].as<float>();
			return true;
		}
	};


};


YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& value);
YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& value);
YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& value);
YAML::Emitter& operator<<(YAML::Emitter& out, const SubTexture& a);

