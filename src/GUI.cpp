
#include "GUI.h"

#include "glutils.h"

#include "shader.h"

GuiState::GuiState(ogl::window* winp) {
	this->win = winp;
}

void GuiState::convert_rect(rectf& rec) {
	rec.y = win->size.y - rec.y;
}

bool GuiState::inside(const rectf& rec) {
	vec2f cur = win->cursor();
	gui_active |= item_howered;
	return rec.x < cur.x && rec.y < cur.y && rec.x + rec.z > cur.x && rec.y + rec.w > cur.y;
}

bool GuiState::pushed(const rectf& rec) {
	return inside(rec) && win->rmb();
}

void GuiState::Icon(rectf rect, const char* IconId) {
	GLuint tex = get_tex(IconId);
	if (tex) {
		win->set_viewport(rect);
		draw_texture(0, tex);
	}
}

bool GuiState::button(rectf rect, const char* name, const char* IconId) {

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

bool GuiState::pupup(rectf rect, float safe_padding) {
	rect.x -= safe_padding;
	rect.y -= safe_padding;
	rect.z += safe_padding * 2;
	rect.w += safe_padding * 2;
	item_howered = inside(rect);
	return item_howered;
}

void GuiState::FloatSlider(rectf rect, float& val, float min, float max) {

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
	rectf controll_rec = rectf(rect.x + pos * (rect.z - controll_size), rect.y + 5, controll_size, rect.w - 10);

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


void GuiState::DrawCircleFilled(const rectf& rect, const rgba& col) {

	glViewport(0, 0, win->size.x, win->size.y);
	DrawCircleFilled({ rect.x + rect.z / 2, rect.y + rect.w / 2 }, rect.w / 2, col);
}

void GuiState::DrawCircleFilled(vec2f pos, float rad, const rgba& col) {
	
	static alni precision = 41;

	glColor4f(col.r, col.g, col.b, col.a);

	double twicePi = 2.0 * 3.142;

	glBegin(GL_TRIANGLE_FAN);

	pos.x = ((pos.x / win->size.x) - 0.5) * 2;
	pos.y = ((pos.y / win->size.y) - 0.5) * 2;

	glVertex3f(pos.x, pos.y, -0.99);

	for (alni i = 0; i <= precision; i++) {
		float facx = 2 * rad / win->size.x;
		float facy = 2 * rad / win->size.y;

		float x = (trigs::cos(i * twicePi / precision)) * facx / 2;
		float y = (trigs::sin(i * twicePi / precision)) * facy / 2;

		x = pos.x + x;
		y = pos.y + y;

		glVertex3f(x, y, -0.99);
	}
	glEnd();
}

void GuiState::ColorPicker(rectf rect, rgba& col) {

	if (inside(rect)) {
		rect.x -= 3;
		rect.y -= 3;
		rect.z += 6;
		rect.w += 6;
	}

	bool active = glfwGetMouseButton(win->winp, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
	vec2f center = vec2f((rect.x + rect.z / 2), (rect.y + rect.w / 2));
	vec2f curs = win->cursor();

	float dot_size = 20;
	float dot_padding = 30;
	float sv_size = rect.z / 2;
	rectf hs_edit_rec = rectf(rect.x + (rect.z - sv_size) / 2, rect.y + (rect.w - sv_size) / 2, sv_size, sv_size);
	
	hsv hsvin = col.rgbs;
	float angle = hsvin.h;

	if (active) {
		if (inside(rect)) {
			if (inside(hs_edit_rec)) {
				hsvin.s = ((curs.x - hs_edit_rec.x) / hs_edit_rec.z);
				hsvin.v = ((curs.y - hs_edit_rec.y) / hs_edit_rec.w);
				
				CLAMP(hsvin.s, 0, 1);
				CLAMP(hsvin.v, 0, 1);
			}
			else {
				angle = trigs::atan2((curs.y - center.y), (curs.x - center.x));
				hsvin.h = angle > 0 ? angle : 2 * PI + angle;
			}
		}
	}

	col = hsvin;

	vec2f dot_pos = vec2f((rect.z - dot_padding) * trigs::cos(angle) / 2, (rect.w - dot_padding) * trigs::sin(angle) / 2);
	rectf dot_rec = rectf(center.x + dot_pos.x - dot_size / 2, center.y + dot_pos.y - dot_size / 2, dot_size, dot_size);
	rectf vs_dot_rec = rectf(hs_edit_rec.x + hs_edit_rec.z * hsvin.s - dot_size / 2, hs_edit_rec.y + hs_edit_rec.w * hsvin.v - dot_size / 2, dot_size, dot_size);
	
	if (inside(rect)) {
		if (inside(hs_edit_rec)) {
			vs_dot_rec.x -= 3;
			vs_dot_rec.y -= 3;
			vs_dot_rec.z += 6;
			vs_dot_rec.w += 6;
		}
		else {
			dot_rec.x -= 3;
			dot_rec.y -= 3;
			dot_rec.z += 6;
			dot_rec.w += 6;
		}
	}

	DrawCircleFilled(dot_rec, rgba(1));
	DrawCircleFilled(vs_dot_rec, rgba(1));

	glViewport(hs_edit_rec.x, hs_edit_rec.y, hs_edit_rec.z, hs_edit_rec.w);

	rgb hue_preview_col = hsv(hsvin.h, 1, 1);
	glBegin(GL_QUADS);
	glColor4f(1, 1, 1, 1); glVertex3f(-1.f, 1.f, -0.99);
	glColor4f(0, 0, 0, 1); glVertex3f(-1.f, -1.f, -0.99);
	glColor4f(0, 0, 0, 1); glVertex3f(1.f, -1.f, -0.99);
	glColor4f(hue_preview_col.r, hue_preview_col.g, hue_preview_col.b, 1); glVertex3f(1.f, 1.f, -0.99);
	glEnd();

	Icon(rect, "../rsc/icons/ColorPickerRGB.png");
}

GuiState::~GuiState() {
}
