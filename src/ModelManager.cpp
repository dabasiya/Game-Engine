#include "ModelManager.h"

std::unordered_map<std::string, std::shared_ptr<Model>> ModelManager::s_PathToModel;


std::shared_ptr<Model> ModelManager::GetModel(const std::string& path) {
	
	if (s_PathToModel.find(path) == s_PathToModel.end()) {
		s_PathToModel[path] = std::make_shared<Model>(path);
	}

	return s_PathToModel[path];
}