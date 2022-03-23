
#pragma once

#include "strings.h"


#include "window.h"

class GuiState {
	ogl::window* win;
	void convert_rect(vec4& rec);

	bool inside(const vec4& rec);
	bool pushed(const vec4& rec);

public:

	bool gui_active = false;
	bool item_howered = false;

	GuiState(ogl::window* winp);

	void Icon(vec4 rect, const char* IconId);

	bool button(vec4 rect, const char* name, const char* IconId);

	bool pupup(vec4 rect, float safe_padding);

	void FloatSlider(vec4 rect, float& val, float min, float max);
	void ColorPicker(vec4 rect, vec4& col);
	bool val_sat_edit(vec4 rect, vec4& col);
	~GuiState();
};