#pragma once

#include <unordered_map>
#include <Model.h>

struct ModelManager {

	static std::shared_ptr<Model> GetModel(const std::string& path);

	static std::unordered_map<std::string, std::shared_ptr<Model>> s_PathToModel;
};