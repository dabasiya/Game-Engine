#include "ShaderManager.h"

std::unordered_map<std::string, std::shared_ptr<Shader>> ShaderManager::s_shadermap;
std::vector<std::string> ShaderManager::s_shadernames;

void ShaderManager::Add(const std::string& name, const std::string& vertexfilepath, const std::string& fragmentfilepath) {
	s_shadermap[name] = std::make_shared<Shader>(vertexfilepath.c_str(), fragmentfilepath.c_str());
	s_shadernames.push_back(name);
}

void ShaderManager::Add(const std::string& name, const std::string& vertexshaderpath, const std::string& geometryshaderpath, const std::string& fragmentshaderpath) {
	s_shadermap[name] = std::make_shared<Shader>(vertexshaderpath.c_str(), geometryshaderpath.c_str(), fragmentshaderpath.c_str());
	s_shadernames.push_back(name);
}

void ShaderManager::Remove(const std::string& name) {
	s_shadermap[name]->Delete();

	s_shadernames.erase(std::remove(s_shadernames.begin(), s_shadernames.end(), name), s_shadernames.end());

	s_shadermap[name].reset();
}

std::shared_ptr<Shader> ShaderManager::Get(const std::string& name) {
	return s_shadermap[name];
}