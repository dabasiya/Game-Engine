#include "SoundManager.h"

#include <iostream>

SoundManager::SoundManager() {
	FMOD::System_Create(&m_system);
	m_system->init(512, FMOD_INIT_NORMAL, nullptr);

	std::cout << "work proper" << std::endl;
}

SoundManager::~SoundManager() {

	for (auto &channel : m_channels) {
		channel.second->stop();
	}

	for (auto& sound : m_sounds) {
		sound.second->release();
	}

	m_system->close();
	m_system->release();
}

void SoundManager::Update() {
	m_system->update();
}

void SoundManager::PlayAudio(const std::string& filename, AudioType audiotype) {
	bool isplaying = false;
	if (m_sounds.find(filename) == m_sounds.end()) {
		FMOD::Sound* t_sound;
		FMOD::Channel* t_channel;

		m_system->createSound(filename.c_str(), audiotype, nullptr, &t_sound);
		m_system->playSound(t_sound, nullptr, false, &t_channel);

		m_channels[filename] = t_channel;
		m_sounds[filename] = t_sound;
	}
	else {
		m_channels[filename]->isPlaying(&isplaying);
		if(!isplaying)
			m_system->playSound(m_sounds[filename], nullptr, false, &m_channels[filename]);
	}
}

void SoundManager::StopAudio(const std::string& filename) {
	if (m_sounds.find(filename) != m_sounds.end()) {
		m_channels[filename]->stop();
	}

	else {
          //std::cout << "Sound File : " << filename << " not exist!" << std::endl;
	}
}

bool SoundManager::isPlaying(const std::string& filename) {
	if (m_channels.find(filename) == m_channels.end()) {
		std::cout << "Sound file : " << filename << " not exist!";
	}
	else {
		bool isplaying = false;
		m_channels[filename]->isPlaying(&isplaying);
		return isplaying;
	}
	return false;
}
