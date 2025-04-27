#pragma once

#include "event.h"

struct WindowResizedEvent : Event {

	unsigned int width = 0;
	unsigned int height = 0;

	WindowResizedEvent(unsigned int a_width, unsigned int a_height)
		: width(a_width), height(a_height)
	{
		Flag = BIT(Event_Type::Window_Resized);
	}
};

struct WindowClosedEvent : Event {
	WindowClosedEvent() {
		Flag = BIT(Event_Type::Window_Closed);
	}
};


