#include "syaml.h"


YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& value) {
	out << YAML::Flow;
	out << YAML::BeginSeq << value.x << value.y << value.z << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& value) {
	out << YAML::Flow;
	out << YAML::BeginSeq << value.x << value.y << value.z << value.w << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& value) {
	out << YAML::Flow;
	out << YAML::BeginSeq << value.x << value.y << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const SubTexture& s) {
	out << YAML::BeginMap;
	out << YAML::Key << "index" << s.index;
	out << YAML::Key << "coords1" << s.coords1;
	out << YAML::Key << "coords2" << s.coords2;
	out << YAML::EndMap;
	return out;
}
