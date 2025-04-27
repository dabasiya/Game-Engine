#include "animationutil.h"

#include <fstream>

#include <syaml.h>
#include <iostream>


Animation readanimation(const std::string& filepath) {
	Animation result;

	std::ifstream ifl(filepath);
	std::stringstream sstream;
	sstream << ifl.rdbuf();

	YAML::Node data = YAML::Load(sstream.str());
	uint32_t index = 0;
	while (true) {
		std::string strindex = std::to_string(index);
		if (!data[strindex.c_str()])
			break;
		auto frame = data[strindex.c_str()];
		result.Append({ frame["index"].as<uint32_t>(),
		frame["coords1"].as<glm::vec2>(),
		frame["coords2"].as<glm::vec2>(),
		frame["xmultiplier"].as<float>(),
		frame["ymultiplier"].as<float>()
			}, frame["time"].as<float>());
		index++;
	}
	return result;
}


void writeanimation(const std::string& filepath, Animation m_animation) {
	YAML::Emitter out;
	out << YAML::BeginMap;
	for (unsigned int i = 0; i < m_animation.frames.size(); i++) {
		std::string number = std::to_string(i);
		out << YAML::Key << number << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "index" << YAML::Value << m_animation.frames[i].index;
		out << YAML::Key << "coords1" << YAML::Value << m_animation.frames[i].coords1;
		out << YAML::Key << "coords2" << YAML::Value << m_animation.frames[i].coords2;
		out << YAML::Key << "xmultiplier" << YAML::Value << m_animation.frames[i].X_Multiplier;
		out << YAML::Key << "ymultiplier" << YAML::Value << m_animation.frames[i].Y_Multiplier;
		out << YAML::Key << "time" << YAML::Value << m_animation.frametimes[i];
		out << YAML::EndMap;
	}
	out << YAML::EndMap;
	std::ofstream of1(filepath);
	of1 << out.c_str();
}
