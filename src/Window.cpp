#include "window.h"

float Window::Ratio = 1.0f;

float Window::Mousex = 0.0f;
float Window::Mousey = 0.0f;

float Window::Last_Mousex = 0.0f;
float Window::Last_Mousey = 0.0f;

double Window::Window_Mousex = 0.0;
double Window::Window_Mousey = 0.0;

double Window::Window_Last_Mousex = 0.0;
double Window::Window_Last_Mousey = 0.0;


// window ID object
GLFWwindow* Window::ID;

// set orthographic camera size
float Window::OrthographicSize = 2.0f;

// set static variable values

unsigned int Window::Width = 0;
unsigned int Window::Height = 0;

bool Window::Resized = false;

float Window::UI_Size = 1.0f;

EventCallBack Window::EventCallBackFunction;




void Window::SetEventCallBack(EventCallBack e) {
	EventCallBackFunction = e;
}

Window::Window(const std::string& a_title, unsigned int a_width, unsigned int a_height)
	: Title(a_title)
{
	Width = a_width;
	Height = a_height;
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	ID = glfwCreateWindow(Width, Height, Title.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(ID);
	glfwSwapInterval(1);
	// initalize event callback methods and other stuff
	Init();
}

void Window::SwapBuffers() {
	glfwPollEvents();
	glfwSwapBuffers(ID);
}



void Window::Init() {

	Ratio = (float)Width / (float)Height;

	glfwSetFramebufferSizeCallback(ID, this->WindowResizedEventFn);

	glfwSetMouseButtonCallback(ID, this->MouseEventFn);

	glfwSetInputMode(ID, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glfwSetCursorPosCallback(ID, this->MousePosEventFn);

	glfwSetCharCallback(ID, this->KeyTypedEventFn);

	glfwSetKeyCallback(ID, this->KeyEventFn);

	glfwSetScrollCallback(ID, this->MouseScrollEventFn);
}
// return true if window should be close

bool Window::ShouldClose() {
	return glfwWindowShouldClose(ID);
}

// for destroy window
void Window::Destroy() {
	glfwDestroyWindow(ID);
}



// for reset all event happened flags
// and do last cursor position

void Window::Update_MousePositions() {

	Window_Last_Mousex = Window_Mousex;
	Window_Last_Mousey = Window_Mousey;

	Last_Mousex = Mousex;
	Last_Mousey = Mousey;
}

// event handling functions


void Window::WindowResizedEventFn(GLFWwindow* window, int a_width, int a_height) {
	Width = (unsigned int)a_width;
	Height = (unsigned int)a_height;
	Resized = true;

	Ratio = (float)Width / (float)Height;

	// updated
	WindowResizedEvent e(a_width, a_height);
	EventCallBackFunction(e);
}

void Window::MouseEventFn(GLFWwindow* window, int button, int action, int mods) {

	if (action == GLFW_PRESS) {
		MousePressedEvent e(button);
		EventCallBackFunction(e);
	}
	else if (action == GLFW_RELEASE) {
		MouseReleasedEvent e(button);
		EventCallBackFunction(e);
	}
}

void Window::MousePosEventFn(GLFWwindow* window, double x, double y) {


	Window_Mousex = x;
	Window_Mousey = y;


	Mousex = (x / Width) * OrthographicSize * Ratio;
	Mousex -= (OrthographicSize * Ratio / 2);
	Mousey = (y / Height) * OrthographicSize;
	Mousey = -(Mousey - (OrthographicSize / 2));

	MouseMovedEvent e((float)x, (float)y);
	EventCallBackFunction(e);
}


void Window::KeyTypedEventFn(GLFWwindow* window, unsigned int keycode) {
	KeyTypedEvent e(keycode);
	EventCallBackFunction(e);
}

void Window::KeyEventFn(GLFWwindow* window, int keycode, int scancode, int action, int modes) {

	switch (action) {
	case GLFW_PRESS:
	{
		KeyPressedEvent e(keycode, modes);
		EventCallBackFunction(e);
		break;
	}
	case GLFW_RELEASE:
	{
		KeyReleasedEvent e(keycode);
		EventCallBackFunction(e);
		break;
	}
	}

}

void Window::MouseScrollEventFn(GLFWwindow* window, double offsetx, double offsety) {
	MouseScrolledEvent e(offsetx, offsety);
	EventCallBackFunction(e);
}