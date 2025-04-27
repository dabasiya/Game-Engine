#pragma once

#include <GLFW/glfw3.h>
#include <string>

#include <Event/Event.h>
#include <Event/WindowEvents.h>
#include <Event/MouseEvents.h>
#include <Event/KeyEvents.h>

#include <functional>



// temporary
#include <iostream>


using EventCallBack = std::function<void(Event&)>;

struct Window {

	static float UI_Size;

	// ratio ; width / height
	static float Ratio;

	// mouse position in floats gl
	static float Mousex;
	static float Mousey;

	static float Last_Mousex;
	static float Last_Mousey;


	// mouse position as per window pixel
	static double Window_Mousex;
	static double Window_Mousey;

	// last mouse position as per window pixel
	static double Window_Last_Mousex;
	static double Window_Last_Mousey;


	// window pointer
	static GLFWwindow* ID;


	// window title
	std::string Title;
	static unsigned int Width;
	static unsigned int Height;


	// orthographic camera size
	// i use orthographic because this is 2d game only

	static float OrthographicSize;

	// event happened flags
	static bool Resized;


	static EventCallBack EventCallBackFunction;

	// for set event callback function
	// this must be set before handling any events

	Window(const std::string& a_title, unsigned int a_width, unsigned int a_height);

	void SwapBuffers();

	void SetEventCallBack(EventCallBack e);

	void Init();

	bool ShouldClose();

	void Destroy();

	static void Update_MousePositions();

	static void WindowResizedEventFn(GLFWwindow* window, int a_width, int a_height);

	static void MouseEventFn(GLFWwindow* window, int button, int action, int mods);

	static void MousePosEventFn(GLFWwindow* window, double x, double y);

	static void KeyTypedEventFn(GLFWwindow* window, unsigned int keycode);

	static void KeyEventFn(GLFWwindow* window, int keycode, int scancode, int action, int modes);

	static void MouseScrollEventFn(GLFWwindow* window, double offsetx, double offsety);
};
