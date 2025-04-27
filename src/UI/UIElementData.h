#pragma once

#include <glm/glm.hpp>


// elements data for drawing and input etc for custom ui

#define WINDOW_TAB_HEIGHT 30
#define WINDOW_CLOSE_BUTTON_SIZE 25

// style
enum uistyle {
	row_two_block = 0
};

struct windowdata {

	windowdata(int a_x, int a_y, int a_width, const glm::vec4& a_tab_color, const glm::vec4& a_panel_color)
		: x(a_x), y(a_y), width(a_width), tab_color(a_tab_color), panel_color(a_panel_color), content_offset_x(0), content_offset_y(WINDOW_TAB_HEIGHT)
	{
	}

	int x, y;

	int width;

	bool visible = false;

  int offsety = 0;

	int content_offset_x, content_offset_y;

	// z coordinate for if window selected it appear first
	float z = 0.0f;

        unsigned int mouse_lock_x, mouse_lock_y;

        bool mouse_locked = false;

        int storex, storey;

  unsigned int maxheight = 0;

	glm::vec4 tab_color = glm::vec4(1.0f);
	glm::vec4 panel_color = glm::vec4(1.0f);
};


