
#pragma once

#include "strings.h"
#include "map.h"

#include "window.h"

class GuiState {

	HashMap<GLuint, string> textures;
	ogl::window* win;

	GLuint get_tex(const char* TexId);
	void convert_rect(vec4& rec);

	bool inside(const vec4& rec);
	bool pushed(const vec4& rec);

public:

	GuiState(ogl::window* winp);

	void Icon(vec4 rect, const char* IconId);

	bool button(vec4 rect, const char* name, const char* IconId);

	~GuiState();
};