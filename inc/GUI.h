
#pragma once

#include "strings.h"

#include "window.h"

class GuiState {
	ogl::window* win;
	void convert_rect(rectf& rec);

	bool inside(const rectf& rec);
	bool pushed(const rectf& rec);

public:

	bool gui_active = false;
	bool item_howered = false;

	GuiState(ogl::window* winp);

	void Icon(rectf rect, const char* IconId);

	bool button(rectf rect, const char* name, const char* IconId);

	bool pupup(rectf rect, float safe_padding);

	void FloatSlider(rectf rect, float& val, float min, float max);
	void ColorPicker(rectf rect, rgba& col);
	
	void DrawCircleFilled(const rectf& rect, const rgba& col);
	void DrawCircleFilled(vec2f pos, float rad, const rgba& col);

	~GuiState();
};