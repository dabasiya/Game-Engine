#pragma once

#include <fstream>
#include <glm/glm.hpp>


struct Shader {
	unsigned int ID;

	Shader(const char* vertexpath, const char* fragmentpath);

	void Bind();

	std::string ReadFile(const char* path);

	// functions set uniform variable in shaders

	int GetLocation(const char* name);

	void SetMat4(const char* name, const glm::mat4& value);
	void SetVec3(const char* name, const glm::vec3& value);
	void SetInt(const char* name, int value);


};
