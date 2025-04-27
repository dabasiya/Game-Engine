#pragma once

#define BIT(x) 1<<x

enum Event_Type {
	Window_Resized = 0,
	Window_Closed = 1,
	Key_Pressed = 2,
	Key_Released = 3,
	Key_Typed = 4,
	Mouse_Pressed = 5,
	Mouse_Released = 6,
	Mouse_Moved = 7,
	Mouse_Scrolled = 8
};

struct Event {
	unsigned int Flag = 0;

	bool is(Event_Type e) {
		return (bool)(Flag & BIT(e));
	}
};

