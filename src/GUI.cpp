
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
	gui_active |= item_howered;
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

	item_howered = inside(rect);
	
	if (item_howered) {
		rect.x -= 5;
		rect.y -= 5;
		rect.z += 10;
		rect.w += 10;
	}

	bool pressed = pushed(rect);

	if (win->pen_pressure() && item_howered) {
		rect.x += 10;
		rect.y += 10;
		rect.z -= 20;
		rect.w -= 20;
	}

	Icon(rect, IconId);

	return pressed;
}

bool GuiState::pupup(vec4 rect, float safe_padding) {
	rect.x -= safe_padding;
	rect.y -= safe_padding;
	rect.z += safe_padding * 2;
	rect.w += safe_padding * 2;
	item_howered = inside(rect);
	return item_howered;
}

void GuiState::FloatSlider(vec4 rect, float& val, float min, float max) {

	item_howered = inside(rect);

	if (item_howered) {
		rect.x -= 3;
		rect.y -= 3;
		rect.z += 6;
		rect.w += 6;
	}

	CLAMP(val, min, max);
	float pos = val / (max - min);
	float controll_size = rect.z / 14;
	vec4 controll_rec = vec4(rect.x + pos * (rect.z - controll_size), rect.y + 5, controll_size, rect.w - 10);

	if (inside(controll_rec)) {
		controll_rec.x -= 2;
		controll_rec.z += 4;
	}

	Icon(controll_rec, "A:/src/ogl/rsc/icons/SliderControl.png");
	Icon(rect, "A:/src/ogl/rsc/icons/Slider.png");
	
	if (glfwGetMouseButton(win->winp, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
		if (item_howered) {
			val = ((win->cursor().x - rect.x) / rect.z) * (max - min);
			CLAMP(val, min, max);
		}
	}
}


float col_to_angle(vec4 col) {
	float angle = 0;
	int state = 0;
	float rng = 2 * PI / 3;

	if (!col.r && !col.g && !col.b) {
		return 0;
	}

	if (col.r > col.g && col.r > col.b) {
		state = (col.g > col.b) ? 1 : 2;
	}
	else if (col.g > col.r && col.g > col.b) {
		state = (col.r > col.b) ? 1 : 3;
	}
	else {
		state = (col.r > col.g) ? 2 : 3;
	}

	switch (state) {
		case 1:  // rg 
			return (col.g / (col.g + col.r)) * rng;
		case 2:  // br 
			return rng * 2 + (col.r / (col.b + col.r)) * rng;
		case 3: // gb 
			return rng + (col.b / (col.g + col.b)) * rng;
	}
	return angle;
}

vec4 angle_to_col(float angle) {
	float rng = 2 * PI / 3;
	float interp = 0;
	vec4 out;
	if (angle > rng * 2) { // blue -> red
		interp = (angle - (rng * 2)) / rng;
		out.r = interp;
		out.g = 0;
		out.b = 1 - interp;

	}
	else if (angle > rng) { // green -> blue
		interp = (angle - (rng * 1)) / rng;
		out.r = 0;
		out.g = 1 - interp;
		out.b = interp;
	}
	else { // red -> green
		interp = angle / rng;
		out.r = 1 - interp;
		out.g = interp;
		out.b = 0;
	}
	
	out.a = 1;
	return out;
}

float get_val(vec4& out) {
	return glm::max(glm::max(out.r, out.g), out.b);
}

void apply_val(vec4& out, float val) {
	float scale = val / get_val(out);
	out.r *= scale;
	out.g *= scale;
	out.b *= scale;
}

float get_saturation(vec4& out) {

}

vec4 apply_saturation(vec4& out, float sat) {

}

bool GuiState::val_sat_edit(vec4 rect, vec4& col) {
	bool active = glfwGetMouseButton(win->winp, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && inside(rect);

	if (active) {
		apply_saturation(col, (rect.x - win->cursor().x) / rect.z);
		apply_val(col, (rect.y - win->cursor().y) / rect.w);
	}

	vec2 dot_pos = vec2(rect.x + get_val(col) * rect.z, rect.y + get_saturation(col) * rect.w);

	return active;
}

void GuiState::ColorPicker(vec4 rect, vec4& col) {

	item_howered = inside(rect);
	if (item_howered) {
		rect.x -= 3;
		rect.y -= 3;
		rect.z += 6;
		rect.w += 6;
	}

	float sv_size = rect.z / 1.5;
	vec4 hs_edit_rec = vec4(rect.x + (rect.z - sv_size) / 2, rect.y + (rect.z - sv_size) / 2, sv_size, sv_size);
	bool hs_active = val_sat_edit(hs_edit_rec, col);
	 

	float angle = col_to_angle(col);

	float dot_size = 20;
	float dot_padding = 30;
	vec2 center = vec2((rect.x + rect.z / 2), (rect.y + rect.w / 2));
	vec2 dot_pos = vec2((rect.z - dot_padding) * cos(angle) / 2, (rect.w - dot_padding) * sin(angle) / 2);
	vec4 dot_rec = vec4(center.x + dot_pos.x - dot_size / 2, center.y + dot_pos.y - dot_size / 2, dot_size, dot_size);

	Icon(dot_rec, "A:/src/ogl/rsc/icons/Dot.png");
	Icon(rect, "A:/src/ogl/rsc/icons/ColorPickerRGB.png");

	if (!hs_active && glfwGetMouseButton(win->winp, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
		if (item_howered) {
			vec2 curs = win->cursor();
			float dx = (curs.x - center.x);
			float dy = (curs.y - center.y);
			angle = atan2(dy, dx);
			angle = angle > 0 ? angle : 2 * PI + angle;
			col = angle_to_col(angle);
		}
	}
}

GuiState::~GuiState() {
}
