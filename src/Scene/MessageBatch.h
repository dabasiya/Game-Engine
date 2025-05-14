#pragma once

#include "Message.h"
#include <vector>


struct MessageBatch {

	std::vector<Message> messages;

	void Add(MessageType type, const std::string& text, float lifetime = 3.0f, float fadetime = 0.5f);


	void Update(float time);
};
