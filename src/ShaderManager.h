#pragma once


#include "Shader.h"
#include <unordered_map>

struct ShaderManager {
	
	static std::vector<std::string> s_shadernames;

	static std::unordered_map<std::string, std::shared_ptr<Shader>> s_shadermap;

	static void Add(const std::string& name, const std::string& vertexshaderpath, const std::string& fragmentshaderpath);
	static void Add(const std::string& name, const std::string& vertexshaderpath, const std::string& geometryshaderpath, const std::string& fragmentshaderpath);
	static void Remove(const std::string& name);
	static std::shared_ptr<Shader> Get(const std::string& name);
};