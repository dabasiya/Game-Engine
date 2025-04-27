#pragma once

#include "Event.h"

struct KeyPressedEvent : Event {
	int key = 0;
	int mod = 0;

	KeyPressedEvent(int a_key, int a_mod)
		: key(a_key), mod(a_mod)
	{
		Flag = BIT(Event_Type::Key_Pressed);
	}
};


struct KeyReleasedEvent : Event {
	int key = 0;

	KeyReleasedEvent(int a_key)
		: key(a_key)
	{
		Flag = BIT(Event_Type::Key_Released);
	}
};


struct KeyTypedEvent : Event {
	unsigned int key = 0;

	KeyTypedEvent(unsigned int a_key)
		: key(a_key)
	{
		Flag = BIT(Event_Type::Key_Typed);
	}
};

