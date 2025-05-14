#pragma once

#include "Event.h"


struct MousePressedEvent : Event {
	int Button = 0;

	MousePressedEvent(int a_button)
		: Button(a_button)
	{
		Flag = BIT(Event_Type::Mouse_Pressed);
	}
};


struct MouseReleasedEvent : Event {
	int Button = 0;

	MouseReleasedEvent(int a_button)
		: Button(a_button)
	{
		Flag = BIT(Event_Type::Mouse_Released);
	}
};


struct MouseMovedEvent : Event {
	float x = 0.0f;
	float y = 0.0f;

	MouseMovedEvent(float a_x, float a_y)
		: x(a_x), y(a_y)
	{
		Flag = BIT(Event_Type::Mouse_Moved);
	}
};

struct MouseScrolledEvent : Event {
	float xoffset = 0.0f;
	float yoffset = 0.0f;

	MouseScrolledEvent(float a_xoffset, float a_yoffset)
		: xoffset(a_xoffset), yoffset(a_yoffset)
	{
		Flag = BIT(Event_Type::Mouse_Scrolled);
	}
};
