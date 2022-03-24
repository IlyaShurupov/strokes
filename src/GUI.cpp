
#include "GUI.h"

#include "glutils.h"

#include "shader.h"

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

	Icon(controll_rec, "../rsc/icons/SliderControl.png");
	Icon(rect, "../rsc/icons/Slider.png");
	
	if (glfwGetMouseButton(win->winp, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
		if (item_howered) {
			val = ((win->cursor().x - rect.x) / rect.z) * (max - min);
			CLAMP(val, min, max);
		}
	}
}


typedef struct {
	double r;       // a fraction between 0 and 1
	double g;       // a fraction between 0 and 1
	double b;       // a fraction between 0 and 1
} rgb;
typedef struct {
	double h;       // angle in degrees
	double s;       // a fraction between 0 and 1
	double v;       // a fraction between 0 and 1
} hsv;
hsv rgb2hsv(rgb in) {
	hsv         out;
	double      min, max, delta;

	min = in.r < in.g ? in.r : in.g;
	min = min < in.b ? min : in.b;

	max = in.r > in.g ? in.r : in.g;
	max = max > in.b ? max : in.b;

	out.v = max;                                // v
	delta = max - min;
	if (delta < 0.00001)
	{
		out.s = 0;
		out.h = 0; // undefined, maybe nan?
		return out;
	}
	if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
		out.s = (delta / max);                  // s
	}
	else {
		// if max is 0, then r = g = b = 0              
		// s = 0, h is undefined
		out.s = 0.0;
		out.h = NAN;                            // its now undefined
		return out;
	}
	if (in.r >= max)                           // > is bogus, just keeps compilor happy
		out.h = (in.g - in.b) / delta;        // between yellow & magenta
	else
		if (in.g >= max)
			out.h = 2.0 + (in.b - in.r) / delta;  // between cyan & yellow
		else
			out.h = 4.0 + (in.r - in.g) / delta;  // between magenta & cyan

	out.h *= 60.0;                              // degrees

	if (out.h < 0.0)
		out.h += 360.0;

	return out;
}
rgb hsv2rgb(hsv in) {
	double      hh, p, q, t, ff;
	long        i;
	rgb         out;

	if (in.s <= 0.0) {       // < is bogus, just shuts up warnings
		out.r = in.v;
		out.g = in.v;
		out.b = in.v;
		return out;
	}
	hh = in.h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = in.v * (1.0 - in.s);
	q = in.v * (1.0 - (in.s * ff));
	t = in.v * (1.0 - (in.s * (1.0 - ff)));

	switch (i) {
	case 0:
		out.r = in.v;
		out.g = t;
		out.b = p;
		break;
	case 1:
		out.r = q;
		out.g = in.v;
		out.b = p;
		break;
	case 2:
		out.r = p;
		out.g = in.v;
		out.b = t;
		break;

	case 3:
		out.r = p;
		out.g = q;
		out.b = in.v;
		break;
	case 4:
		out.r = t;
		out.g = p;
		out.b = in.v;
		break;
	case 5:
	default:
		out.r = in.v;
		out.g = p;
		out.b = q;
		break;
	}
	return out;
}

void GuiState::ColorPicker(vec4 rect, vec4& col) {

	if (inside(rect)) {
		rect.x -= 3;
		rect.y -= 3;
		rect.z += 6;
		rect.w += 6;
	}

	bool active = glfwGetMouseButton(win->winp, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
	vec2 center = vec2((rect.x + rect.z / 2), (rect.y + rect.w / 2));
	vec2 curs = win->cursor();


	float dot_size = 20;
	float dot_padding = 30;
	float sv_size = rect.z / 2;
	vec4 hs_edit_rec = vec4(rect.x + (rect.z - sv_size) / 2, rect.y + (rect.w - sv_size) / 2, sv_size, sv_size);
	
	hsv hsvin = rgb2hsv({ col.r, col.g, col.b });
	float angle = hsvin.h / 360 * (2 * PI);

	if (active) {
		if (inside(rect)) {
			if (inside(hs_edit_rec)) {
				hsvin.s = ((curs.x - hs_edit_rec.x) / hs_edit_rec.z);
				hsvin.v = ((curs.y - hs_edit_rec.y) / hs_edit_rec.w);
				
				CLAMP(hsvin.s, 0, 1);
				CLAMP(hsvin.v, 0, 1);
			}
			else {
				angle = atan2((curs.y - center.y), (curs.x - center.x));
				angle = angle > 0 ? angle : 2 * PI + angle;

				hsvin.h = (angle / (2 * PI)) * 360;
			}
		}
	}

	rgb out = hsv2rgb(hsvin);
	col.r = out.r;
	col.g = out.g;
	col.b = out.b;

	vec2 dot_pos = vec2((rect.z - dot_padding) * cos(angle) / 2, (rect.w - dot_padding) * sin(angle) / 2);
	vec4 dot_rec = vec4(center.x + dot_pos.x - dot_size / 2, center.y + dot_pos.y - dot_size / 2, dot_size, dot_size);
	vec4 vs_dot_rec = vec4(hs_edit_rec.x + hs_edit_rec.z * hsvin.s - dot_size / 2, hs_edit_rec.y + hs_edit_rec.w * hsvin.v - dot_size / 2, dot_size, dot_size);
	
	Icon(dot_rec, "../rsc/icons/Dot.png");
	Icon(vs_dot_rec, "../rsc/icons/Dot.png");
	Icon(hs_edit_rec, "../rsc/icons/HSV.png");
	Icon(rect, "../rsc/icons/ColorPickerRGB.png");
}

GuiState::~GuiState() {
}
