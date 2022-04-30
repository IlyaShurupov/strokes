
#include "app.h"

#include "glutils.h"

#include "shader.h"

bool StrokeApp::pushed(const rectf& rec) {
	return rec.inside(window.cursor()) && window.rmb();
}

bool StrokeApp::button(rectf& rect) {

	item_howered = rect.inside(window.cursor());

	if (item_howered) {
		rect.pos -= 4;
		rect.size += 8;
	}

	bool pressed = pushed(rect);

	if (window.pen_pressure() && item_howered) {
		rect.pos += 6;
		rect.size -= 12;
	}

	DrawRectF(rectf(rect.x - 2, rect.y - 2, rect.w + 4, rect.z + 4), rgba(0.33, 0.33, 0.33, 1), roundness + 3);
	DrawRectF(rect, uicol, roundness);

	if (pressed) {
		need_update = true;
	}

	gui_active |= item_howered;
	return pressed;
}

bool StrokeApp::pupup(rectf rect, float safe_padding) {
	rect.pos -= safe_padding;
	rect.size += safe_padding * 2;
	item_howered = rect.inside(window.cursor());
	gui_active |= item_howered;
	return item_howered;
}

void StrokeApp::FloatSlider(rectf rect, float& val, float min, float max) {

	item_howered = rect.inside(window.cursor());
	gui_active |= item_howered;

	if (item_howered) {
		rect.pos -= 3;
		rect.size += 6;
	}

	CLAMP(val, min, max);
	float pos = val / (max - min);
	float controll_size = rect.z / 14;
	rectf controll_rec = rectf(rect.x + pos * (rect.z - controll_size), rect.y + 5, controll_size, rect.w - 10);

	if (controll_rec.inside(window.cursor())) {
		controll_rec.x -= 2;
		controll_rec.z += 4;
	}

	Texture(controll_rec, "rsc/icons/SliderControl.png");
	Texture(rect, "rsc/icons/Slider.png");

	if (glfwGetMouseButton(window.winp, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
		if (item_howered) {
			val = ((window.cursor().x - rect.x) / rect.z) * (max - min);
			CLAMP(val, min, max);
		}
	}
}


void StrokeApp::ColorPicker(rectf rect, rgba& col) {

	if (rect.inside(window.cursor())) {
		rect.x -= 3;
		rect.y -= 3;
		rect.z += 6;
		rect.w += 6;
	}

	gui_active |= item_howered;

	bool active = glfwGetMouseButton(window.winp, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
	vec2f center = vec2f((rect.x + rect.z / 2), (rect.y + rect.w / 2));
	vec2f curs = window.cursor();

	float sv_size = rect.z / 2;
	rectf hs_edit_rec = rectf(rect.x + (rect.z - sv_size) / 2, rect.y + (rect.w - sv_size) / 2, sv_size, sv_size);

	hsv hsvin = col.rgbs;
	float angle = hsvin.h;

	if (active) {
		if (rect.inside(window.cursor())) {
			if (hs_edit_rec.inside(window.cursor())) {
				hsvin.s = ((curs.x - hs_edit_rec.x) / hs_edit_rec.z);
				hsvin.v = ((curs.y - hs_edit_rec.y) / hs_edit_rec.w);

				CLAMP(hsvin.s, 0, 1);
				CLAMP(hsvin.v, 0, 1);
			} else {
				angle = trigs::atan2((curs.y - center.y), (curs.x - center.x));
				hsvin.h = angle > 0 ? angle : 2 * PI + angle;
			}
		}
	}

	col = hsvin;

	float dot_padding = 30;
	halnf hue_dot_size = 7;
	halnf vs_dot_size = 7;
	vec2f hue_dot_pos = vec2f(rect.x + rect.z / 2.f + (rect.z - dot_padding) * trigs::cos(angle) / 2,
		rect.y + rect.w / 2.f + (rect.w - dot_padding) * trigs::sin(angle) / 2);
	vec2f vs_dot_pos = vec2f(hs_edit_rec.x + hs_edit_rec.z * hsvin.s, hs_edit_rec.y + hs_edit_rec.w * hsvin.v);

	if (rect.inside(window.cursor())) {
		if (hs_edit_rec.inside(window.cursor())) {
			hue_dot_size -= 3;
		} else {
			vs_dot_size -= 3;
		}
	}

	DrawCircleF(hue_dot_pos, hue_dot_size, rgba(1));
	DrawCircleF(vs_dot_pos, vs_dot_size, rgba(1));

	glViewport(hs_edit_rec.x, hs_edit_rec.y, hs_edit_rec.z, hs_edit_rec.w);

	rgb hue_preview_col = hsv(hsvin.h, 1, 1);
	glBegin(GL_QUADS);
	glColor4f(1, 1, 1, 1); glVertex3f(-1.f, 1.f, -0.99);
	glColor4f(0, 0, 0, 1); glVertex3f(-1.f, -1.f, -0.99);
	glColor4f(0, 0, 0, 1); glVertex3f(1.f, -1.f, -0.99);
	glColor4f(hue_preview_col.r, hue_preview_col.g, hue_preview_col.b, 1); glVertex3f(1.f, 1.f, -0.99);
	glEnd();

	Texture(rect, "rsc/icons/ColorPickerRGB.png");
}

void StrokeApp::draw_brush_properties(rectf rect) {
	halnf slider_size = 40;
	halnf picker_size = 180;

	rectf slider_rec = rectf(rect.x, rect.w + rect.y - slider_size, rect.z, slider_size);
	if (sampler.eraser) {
		FloatSlider(slider_rec, sampler.eraser_size, 0.0001f, 0.2f);
		popup_size.y = slider_size;
	} else {
		FloatSlider(slider_rec, sampler.screen_thikness, 0.001f, 0.2f);
		halnf size = MIN(rect.z, rect.w - (slider_size + 10));
		CLAMP(size, 5, 1000);
		ColorPicker(rectf(rect.x + (rect.z - size) / 2, rect.y, size, size), sampler.stroke_col);

		popup_size.y = picker_size + slider_size + 10;
	}

	popup_size.x = picker_size;
}

void StrokeApp::draw_butt_undo_redo(rectf rect, bool flip) {
	halnf offset = rect.z / 7.f;
	DrawTrigF(
		vec2f(rect.x + (flip ? -offset : offset) + (flip * rect.z), rect.y + rect.w / 2),
		vec2f(rect.x + rect.z / 2, rect.y + rect.w - offset),
		vec2f(rect.x + rect.z / 2, rect.y + offset),
		fillcol
	);

	halnf offset_x = rect.z / 5.f;
	offset = rect.z / 3.f;
	DrawRectF(rectf(
		vec2f(rect.x + (flip ? offset_x : (rect.z / 2.f - 1.f)), rect.y + offset),
		vec2f(rect.z / 2.f - offset_x, rect.w - offset * 2)
	), fillcol, 0.f);
}

void StrokeApp::draw_properties() {
	halni butt_size = 40.f;
	halnf prop_size = 450;
	rectf rec = rectf(window.size.x - (show_properties ? prop_size : 70), window.size.y - 70, butt_size, butt_size);
	if (button(rec)) {
		show_properties = !show_properties;
	}

	DrawLine(rec.pos + 10, rec.pos + rec.size - 10, fillcol, 5);
	DrawLine(vec2f(rec.x + rec.z - 10, rec.y + 10), vec2f(rec.x + 10, rec.y + rec.w - 10), fillcol, 5);
	DrawLine(vec2f(rec.x + rec.z / 2, rec.y + 6), vec2f(rec.x + rec.z / 2, rec.y + rec.w - 6), fillcol, 5);
	DrawLine(vec2f(rec.x + 6, rec.y + rec.w / 2), vec2f(rec.x + rec.z - 6, rec.y + rec.w / 2), fillcol, 5);
	DrawCircle(rec.pos + rec.size / 2.f, rec.size.x / 2.f - 15, fillcol, 10);
	DrawCircle(rec.pos + rec.size / 2.f, rec.size.x / 2.f - 17, uicol, 5);

	if (!show_properties) return;

	using namespace ImGui;

	{
		ImGui::SetNextWindowPos(ImVec2(rec.pos.x + rec.z / 2.f + 30, 10));
		ImGui::SetNextWindowSize(ImVec2(prop_size - 60, window.size.y - 20));

		//ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::Begin("Properties", 0, window_flags);
		
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (BeginMenuBar()) {
			Text("Properties");
			EndMenuBar();
		}
	}

	Begin("View");
	if (Button("Reset Camera")) {
		cam.lookat({0, 0, 0}, {100, 0, 0}, {0, 0, 1});
	}
	halnf fov = cam.get_fov();
	SliderFloat("Field Of View", &fov, 0.1, PI - 0.1);
	cam.set_fov(fov);
	End();

	Begin("Canvas");
	ColorPicker4("BackGround Color", &layer.canvas_color.r);
	End();

	Begin("Strokes");
	SliderFloat("Input Precision", &sampler.screen_precision, 0, 0.1);
	Text("Denoising");
	Text("Smothing");
	Text("Deduction");
	End();

	End();

	//ImGui::PopStyleVar();
}

void StrokeApp::draw_toolbar(rectf rect) {

	halni buttlen = 5;
	halnf padding = 10.f;
	halni butt_size = 40.f;
	halnf length = butt_size * buttlen + (padding - 1) * buttlen;

	tool_bar_rect = rectf(window.size.x / 2.f - length / 2.f, window.size.y - 70, length, butt_size);

	Array<rectf> but_rects(buttlen);
	for (auto i : range(buttlen)) {
		but_rects[i] = rectf(tool_bar_rect.x + (padding + butt_size) * i, tool_bar_rect.y, butt_size, butt_size);
	}

	if (button(but_rects[0])) {
		layer.undo();
	}
	draw_butt_undo_redo(but_rects[0]);

	if (button(but_rects[1])) {
		layer.redo();
	}
	draw_butt_undo_redo(but_rects[1], true);


	if (button(but_rects[2])) {
		sampler.eraser = !sampler.eraser;
	}
	rectf& rec = but_rects[2];
	if (sampler.eraser) {
		DrawLine(rec.pos + 6, rec.pos + rec.size - 6, fillcol, 5);
		DrawLine(vec2f(rec.x + rec.z - 6, rec.y + 6), vec2f(rec.x + 6, rec.y + rec.w - 6), fillcol, 5);
	} else {
		DrawLine(vec2f(rec.x + rec.z / 2, rec.y + 6), vec2f(rec.x + rec.z / 2, rec.y + rec.w - 6), fillcol, 5);
		DrawLine(vec2f(rec.x + 6, rec.y + rec.w / 2), vec2f(rec.x + rec.z - 6, rec.y + rec.w / 2), fillcol, 5);
	}

	bool activator_howered = item_howered;
	if (activator_howered) {
		tollbar_popup = true;
	}

	if (tollbar_popup) {
		rectf& butrec = but_rects[2];
		rectf poup_rec = rectf(butrec.x + butrec.z / 2 - popup_size.x / 2, butrec.y - 15 - popup_size.y, popup_size.x, popup_size.y);

		bool should_close = !pupup(poup_rec, 40);

		if (should_close && !activator_howered) {
			tollbar_popup = false;
		} else {
			draw_brush_properties(poup_rec);
		}
	}

	if (button(but_rects[3])) {
		alni len = layer.strokes.Len();
		for (alni idx = 0; idx < len; idx++) {
			layer.undo();
		}
	}
	rec = but_rects[3];
	DrawLine(vec2f(rec.x + rec.z / 2, rec.y + 6), vec2f(rec.x + rec.z / 2, rec.y + rec.w - 15), fillcol, 20);
	DrawLine(vec2f(rec.x + 6, rec.y + rec.w / 2 + 10), vec2f(rec.x + rec.z - 6, rec.y + rec.w / 2 + 10), fillcol, 5);

	if (button(but_rects[4])) {
		window.post_quit_event();
	}

	DrawCircle(but_rects[4].pos + but_rects[4].size / 2.f, but_rects[4].size.x / 2.f - 5, fillcol, 4);
	DrawLine(vec2f(but_rects[4].x + but_rects[4].z / 2, but_rects[4].y + but_rects[4].w / 2.f),
		vec2f(but_rects[4].x + but_rects[4].z / 2, but_rects[4].y + but_rects[4].w), uicol, 16);

	DrawLine(vec2f(but_rects[4].x + but_rects[4].z / 2, but_rects[4].y + but_rects[4].w / 2.f),
		vec2f(but_rects[4].x + but_rects[4].z / 2, but_rects[4].y + but_rects[4].w - 6), fillcol, 4);
}

void StrokeApp::gui_draw() {
	gui_active = false;

	draw_toolbar(tool_bar_rect);
	draw_properties();

	halnf cur_scale = (sampler.eraser ? sampler.eraser_size : sampler.screen_thikness / 2.f) * window.size.x / 2.f;
	DrawCircle(window.cursor(), cur_scale, rgba(0, 0, 0, 0.7), 4.f);
	DrawCircle(window.cursor(), cur_scale, sampler.stroke_col, 2.f);
}