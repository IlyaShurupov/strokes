
#pragma once

#include "strings.h"


#include "window.h"

class GuiState {
	ogl::window* win;
	void convert_rect(vec4& rec);

	bool inside(const vec4& rec);
	bool pushed(const vec4& rec);

public:

	GuiState(ogl::window* winp);

	void Icon(vec4 rect, const char* IconId);

	bool button(vec4 rect, const char* name, const char* IconId);

	~GuiState();
};