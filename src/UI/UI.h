#pragma once


#include <stack>
#include <string>
#include "UIElementData.h"
#include <FontRenderer.h>
#include <vector>
#include <Event/Event.h>
#include <unordered_map>

enum InputType {
	FLOAT = 0,
	TEXT,
	INT
};

struct InputState {
	std::string text;
	InputType type;
	bool active = false;
};

enum MouseButtonClick {
	RIGHT_CLICK, 
	LEFT_CLICK
};


struct ui {

	// panelsize = ui windows upper bar height
	// elementsize = ui elements height
	// fontsize = font size inside uiwindow
	static int panelsize;
	static int elementsize;
	static int fontsize;

	// this is for shifting values so value can be between -1 to 1
	// hor stands for half * orthographicsize * ratio
	// ho stands for half * orthographicsize
	static double hor;
	static double ho;
	// widthor = (1/window_width) * window::orthographicsize * window::ratio ; used for calculate pixel to float
	// and heighto is also like this
	static double widthor;
	static double heighto;

	// was_mouse_pressed for input
	static bool was_mouse_pressed;

	// clipping data
	static std::vector<int> quadindexes;
	static std::vector<glm::vec4> cliprects;

	static bool onevent(Event& e);

	// this vector contain all windowdatas it is used for handling events like moving uiwindows and hiding uiwindows
	static std::vector<windowdata*> windowdatas;

	static std::shared_ptr<FontRenderer> fr;
	// this stack for store windowdata so it can be used for rendering other window content
	static std::stack<windowdata*> windowstack;

	static windowdata* activewindow;

	static uint32_t activeelementid;

	static std::unordered_map<uint32_t, InputState> uiInputStates;

	static glm::vec4* selectedcolor;

	static void resetInputs();

  static void Init();
  
	static bool mousehover(int x, int y, int width, int height);

	// this for styles of uiwindow
	static std::stack<uistyle> stylestack;
	static void pushstyle(uistyle style);
	static void popstyle();

	static void calculatevalues();

	static void windowmove();

	// for start new window
	static void Begin(const char* title, windowdata& data);

	// for end window
	static void End();

	// print content panel background when it not exist in uielement we are going to add in uiwindow
	static void Panel(unsigned int aheight);


	static void updateuiparameters(windowdata* data, int& inputwidth, int& last_content_offsetx, int& last_content_offsety, float& width, float& height);

	// button
	static bool Button(const char* text, MouseButtonClick c = MouseButtonClick::LEFT_CLICK);

	// input box for text input
	static void InputBox(const char* id, std::string& text);

	// State Button this is state Button
	// this hold state on or off
	// you have to give boolean reference with text string
	// this assign given second boolean value to first bool value
	static void StateButton(const char* text, bool& value, bool value2);

	// separator
	static void Separator();

	// Label
	static void Label(const char* text);

	static void FloatInputBox(const char* id, float& value, float onclickvalue = 0.0f);

	static void UIntInputBox(const char* id, unsigned int& value, int onclickvalue = 0);
	static void UInt16InputBox(const char* id, uint16_t& value, int onclickvalue = 0);

	static bool CheckBox(const char* text, bool& value);

	static void OptionSelector(std::vector<const char*> options, unsigned int& selectedoptionindex);

	static bool DropDownButton(const char* text, bool &value, unsigned int uioffset = 0);

	static void Image(SubTexture* image);

  static void ColorPicker();

  static void ColorEdit(glm::vec4& color);

	static bool ImageButton(glm::ivec2 windowpos, glm::ivec2 size, SubTexture& image, float opacity = 1.0f);

	static unsigned int RadioButtonGroup(const std::vector <const char*>& options, std::vector<bool>& checkoptions);

	static uint32_t HashString(const char* str);
};
