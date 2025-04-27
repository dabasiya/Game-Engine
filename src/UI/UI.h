#pragma once


#include <stack>
#include <string>
#include "UIElementData.h"
#include <FontRenderer.h>
#include <vector>
#include <Event/Event.h>

enum inputtype {
	TEXT,
	FLOAT,
	INT
};

struct inputid {
	std::string id;
	inputtype type;
	std::string text;
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
	static float hor;
	static float ho;
	// widthor = (1/window_width) * window::orthographicsize * window::ratio ; used for calculate pixel to float
	// and heighto is also like this
	static float widthor;
	static float heighto;

	// was_mouse_pressed for input
	static bool was_mouse_pressed;

	static bool onevent(Event& e);

	// this vector contain all windowdatas it is used for handling events like moving uiwindows and hiding uiwindows
	static std::vector<windowdata*> windowdatas;

	static FontRenderer* fr;
	// this stack for store windowdata so it can be used for rendering other window content
	static std::stack<windowdata*> windowstack;

	// this store reference to selected input box
	static inputid activeinputbox;

	static bool mousehover(int x, int y, int width, int height);

	// this for styles of uiwindow
	static std::stack<uistyle> stylestack;
	static void pushstyle(uistyle style);
	static void popstyle();

	static void calculatevalues();

	static void windowmove();

	// for start new window
	static void Begin(const std::string& title, windowdata& data);

	// for end window
	static void End();

	// print content panel background when it not exist in uielement we are going to add in uiwindow
	static void Panel(unsigned int aheight);


	static void updateuiparameters(windowdata* data, int& inputwidth, int& last_content_offsetx, int& last_content_offsety, float& width, float& height);

	// button
	static bool Button(const std::string& text);

	// input box for text input
	static void InputBox(const std::string& id, std::string& text);

	// State Button this is state Button
	// this hold state on or off
	// you have to give boolean reference with text string
	// this assign given second boolean value to first bool value
	static void StateButton(const std::string& text, bool& value, bool value2);

	// separator
	static void Separator();

	// Label
	static void Label(const std::string& text);

	static void FloatInputBox(const std::string& id, float& value, float onclickvalue = 0.0f);

	static void UIntInputBox(const std::string& id, unsigned int& value, int onclickvalue = 0);
	static void UInt16InputBox(const std::string& id, uint16_t& value, int onclickvalue = 0);

	static bool CheckBox(const std::string& text, bool& value);

	static void OptionSelector(std::vector<std::string> options, unsigned int& selectedoptionindex);

	static bool DropDownButton(const std::string& text, bool value);

	static void Image(SubTexture* image);
};
