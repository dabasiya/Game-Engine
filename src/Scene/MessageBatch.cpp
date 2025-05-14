#include "MessageBatch.h"

void MessageBatch::Add(MessageType type, const std::string& text, float lifetime, float fadetime) {
	messages.push_back({ type, text, lifetime, fadetime, 0.0f });
}


void MessageBatch::Update(float time) {
	for (auto& msg : messages) {
		msg.timer += time;
	}
	messages.erase(
		std::remove_if(messages.begin(),
			messages.end(),
			[](const Message& m) {
				return m.timer >= m.lifetime;
			}),
		messages.end());

}
