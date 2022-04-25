
#pragma once

#include "strings.h"

#include "window.h"

class GuiState {
	ogl::window* win;
	void convert_rect(vec4f& rec);

	bool inside(const vec4f& rec);
	bool pushed(const vec4f& rec);

public:

	bool gui_active = false;
	bool item_howered = false;

	GuiState(ogl::window* winp);

	void Icon(vec4f rect, const char* IconId);

	bool button(vec4f rect, const char* name, const char* IconId);

	bool pupup(vec4f rect, float safe_padding);

	void FloatSlider(vec4f rect, float& val, float min, float max);
	void ColorPicker(vec4f rect, rgba& col);
	
	void DrawCircleFilled(const vec4f& rect, const rgba& col);
	void DrawCircleFilled(vec2f pos, float rad, const rgba& col);

	~GuiState();
};