
#include "app.h"

StrokeApp::StrokeApp(vec2f size) : ogl(), window(size, ogl::window::FULL_SCREEN), fbo(size, rgba(0)), gui(&window) {
	File db("data.strokes", osfile_openflags::LOAD);
	load(db);

	window.col_clear = rgba(0.2, 0.2, 0.23, 1);
}

void StrokeApp::proc_inputs() {
	if (gui_is_active) {
		return;
	}

	camera_controller();
	sampler.sample(&layer.strokes, &layer.strokes_undo, window.cursor(true), window.pen_pressure(), &cam);

	if (!sampler.active_state() && sampler.has_input()) {
		layer.add_stroke(sampler.get_stroke());
		sampler.clear();
	}
}

void StrokeApp::send_output() {

	window.begin_draw();
	{
		window.clear();

		window.reset_viewport();

		mat4f cammat = (cam.projmat() * cam.viewmat()).transpose();
		sampler.draw(cammat);
		layer.draw(cammat);

		draw_ui();

	} window.end_draw(/*sampler_active*/);
}

alni StrokeApp::run() {
	while (!window.CloseSignal()) {
		proc_inputs();
		send_output();
	}
	return 0;
}

void StrokeApp::camera_controller() {

	static vec2f prevcur = window.cursor(true);
	vec2f cur = window.cursor(true);

	vec2f delta = prevcur - cur;
	vec3f target = cam.get_target();

	if (glfwGetMouseButton(window.geth(), GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
		cam.rotate(delta.x * 5, delta.y * 5);
	}

	if (glfwGetMouseButton(window.geth(), GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
		cam.zoom((prevcur.y + 1.f) / (cur.y + 1.f));
	}

	if (glfwGetKey(window.geth(), GLFW_KEY_SPACE) == GLFW_PRESS) {
		cam.move(cur, prevcur);
	}

	cam.set_ratio(window.aspect_ratio());
	prevcur = cur;
}

struct StrokeApp_SaveHeader {

	char name[10] = {0};
	char version[10] = {0};
	alni nstrokes = 0;

	StrokeApp_SaveHeader(alni nstrokes) {
		memcp(&name, "strokes", slen("strokes") + 1);
		memcp(&version, "0", slen("0") + 1);
		this->nstrokes = nstrokes;
	}

	StrokeApp_SaveHeader() {}
};

void StrokeApp::save(File& file) {
	StrokeApp_SaveHeader head(layer.strokes.Len());
	file.write<StrokeApp_SaveHeader>(&head);

	for (auto stiter : layer.strokes) {
		stroke* str = &stiter.Data();
		file.write<alni>(&str->points.length);
		file.write<rgba>(&str->mesh.color);

		for (auto piter : str->points) {
			file.write<stroke_point>(&piter.data());
		}
	}
}

void StrokeApp::load(File& file) {
	StrokeApp_SaveHeader head;
	file.read<StrokeApp_SaveHeader>(&head);
	if (!memequal(head.name, "strokes", slen("strokes"))) {
		return;
	}

	for (alni str_idx = 0; str_idx < head.nstrokes; str_idx++) {
		stroke str = stroke();

		alni p_len; file.read<alni>(&p_len);
		rgba color; file.read<rgba>(&color);

		str.points.Reserve(p_len);

		for (auto piter : str.points) {
			file.read<stroke_point>(&piter.data());
		}

		str.mesh.color = color;
		layer.add_stroke(str);
	}
}

StrokeApp::~StrokeApp() {
	File db("data.strokes", osfile_openflags::SAVE);
	save(db);
}

void StrokeApp::draw_brush_properties(rectf rect) {
	halnf slider_size = 40;
	halnf picker_size = 180;

	rectf slider_rec = rectf(rect.x, rect.w + rect.y - slider_size, rect.z, slider_size);
	if (sampler.eraser) {
		gui.FloatSlider(slider_rec, sampler.eraser_size, 0.0001f, 0.2f);
		popup_size.y = slider_size;
	} else {
		gui.FloatSlider(slider_rec, sampler.screen_thikness, 0.001f, 0.2f);
		halnf size = MIN(rect.z, rect.w - (slider_size + 10));
		CLAMP(size, 5, 1000);
		gui.ColorPicker(rectf(rect.x + (rect.z - size) / 2, rect.y, size, size), sampler.stroke_col);

		popup_size.y = picker_size + slider_size + 10;
	}

	popup_size.x = picker_size;
}

void StrokeApp::draw_toolbar(rectf rect) {

	tool_bar_rect = rectf(window.size.x / 2.f - 350 / 2.f, window.size.y - 70, 300, 50);

	rectf(*get_rect)(rectf & in) = [](rectf& in) {
		static int idx = 0;
		float butns = 5;
		float item_size = in.z / butns;
		float padding = item_size / 10;
		item_size = item_size - padding;

		rectf out = rectf(in.x + idx * (item_size + padding * 2), in.y, item_size, in.w);

		if (idx == 4) {
			idx = 0;
		} else {
			idx++;
		}
		return out;
	};


	if (gui.button(get_rect(rect), NULL, "../rsc/icons/Backward.png")) {
		layer.undo();
	}
	if (gui.button(get_rect(rect), NULL, "../rsc/icons/Forward.png")) {
		layer.redo();
	}

	{
		rectf butrec = get_rect(rect);
		if (gui.button(butrec, NULL, sampler.eraser ? "../rsc/icons/Eraser.png" : "../rsc/icons/Pen.png")) {
			sampler.eraser = !sampler.eraser;
		}

		bool activator_howered = gui.item_howered;
		if (activator_howered) {
			tollbar_popup = true;
		}

		if (tollbar_popup) {
			rectf poup_rec = rectf(butrec.x + butrec.z / 2 - popup_size.x / 2, butrec.y - 25 - popup_size.y, popup_size.x, popup_size.y);

			bool should_close = !gui.pupup(poup_rec, 40);

			if (should_close && !activator_howered) {
				tollbar_popup = false;
			} else {
				draw_brush_properties(poup_rec);
			}
		}
	}

	if (gui.button(get_rect(rect), NULL, "../rsc/icons/Clear.png")) {
		alni len = layer.strokes.Len();
		for (alni idx = 0; idx < len; idx++) {
			layer.undo();
		}
	}
	if (gui.button(get_rect(rect), NULL, "../rsc/icons/Quit.png")) {
		window.post_quit_event();
	}
}

void StrokeApp::draw_ui() {
	draw_toolbar(tool_bar_rect);

	halnf cur_scale = (sampler.eraser ? sampler.eraser_size : sampler.screen_thikness) * window.size.x;
	rectf cur_rect = rectf(window.cursor().x - cur_scale / 2, window.cursor().y - cur_scale / 2, cur_scale, cur_scale);

	window.set_viewport(cur_rect);
	draw_texture(0, get_tex("../rsc/icons/EraserCursor.png"));

	gui_is_active = gui.gui_active;
	gui.gui_active = false;
}