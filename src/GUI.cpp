
#include "GUI.h"

#include "glutils.h"

GuiState::GuiState(ogl::window* winp) {
	this->win = winp;
}

void GuiState::convert_rect(vec4& rec) {
	rec.y = win->size.y - rec.y;
}

bool GuiState::inside(const vec4& rec) {
	vec2 cur = win->cursor();
	return rec.x < cur.x && rec.y < cur.y && rec.x + rec.z > cur.x && rec.y + rec.w > cur.y;
}

bool GuiState::pushed(const vec4& rec) {
	return inside(rec) && win->rmb();
}

void GuiState::Icon(vec4 rect, const char* IconId) {
	GLuint tex = get_tex(IconId);
	if (tex) {
		draw_texture(0, tex, vec4(0, 0, win->size.x, win->size.y), rect);
	}
}

bool GuiState::button(vec4 rect, const char* name, const char* IconId) {

	bool hovered = inside(rect);
	
	if (hovered) {
		rect.x -= 5;
		rect.y -= 5;
		rect.z += 10;
		rect.w += 10;
	}

	bool pressed = pushed(rect);

	if (win->pen_pressure() && hovered) {
		rect.x += 10;
		rect.y += 10;
		rect.z -= 20;
		rect.w -= 20;
	}

	Icon(rect, IconId);

	return pressed;
}

GuiState::~GuiState() {
}
