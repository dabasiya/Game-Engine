#pragma once

#include <string>

enum MessageType {
        info = 0,
        warning = 1,
        error = 2
};

struct Message {
        MessageType type;
        std::string text;
        float lifetime = 3.0f;
        float fadetime = 0.5f;
        float timer = 0.0f;
};
