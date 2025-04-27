#pragma once

#include <fmod.hpp>
#include <map>
#include <string>

enum AudioType {
	NO_LOOP = FMOD_DEFAULT,
	LOOP = FMOD_LOOP_NORMAL
};

struct SoundManager {
	FMOD::System* m_system = nullptr;

	std::map<std::string, FMOD::Sound*> m_sounds;
	std::map<std::string, FMOD::Channel*> m_channels;

	SoundManager();
	~SoundManager();

	void Update();
	bool isPlaying(const std::string& filename);
	void PlayAudio(const std::string& filename, AudioType audiotype);
	void StopAudio(const std::string& filename);
};