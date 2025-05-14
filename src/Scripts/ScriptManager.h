#pragma once

#include <unordered_map>
#include <string>
#include <iostream>

struct EntityScript;

struct scriptmanager {

	static EntityScript* createscript(const std::string& name) {
		if (scriptmap.find(name) != scriptmap.end()) {
			auto sf = scriptmap[name];
			return sf();
		}
		return nullptr;
	}

	template<typename T>
	static void registerscript(const std::string& name) {
		if (scriptmap.find(name) == scriptmap.end()) {
			scriptmap[name] = []() {
				return static_cast<EntityScript*>(new T());
				};
		}
		else {
			std::cout << "this named script already exist!" << std::endl;
		}
	}


	static std::unordered_map<std::string, EntityScript* (*)()> scriptmap;

	static void Init();

};
