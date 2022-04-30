
#include "app.h"

StrokeApp::StrokeApp(vec2f size) : ImGui::CompleteApp(size, ogl::window::FULL_SCREEN) {
	window.col_clear = rgba(0.2, 0.2, 0.23, 1);
	main_window = false;
	window.minsize.assign(1200, 400);

	cam.lookat({0, 0, 0}, {100, 0, 0}, {0, 0, 1});

	File db("data.strokes", osfile_openflags::LOAD);
	load(db);
}

void StrokeApp::MainProcTick() {
	if (gui_is_active || gui_active) {
		return;
	}

	camera_controller();
	sampler.sample(&layer.strokes, &layer.strokes_undo, window.cursor(1), window.pen_pressure(), &cam);

	if (!sampler.active_state() && sampler.has_input()) {
		layer.add_stroke(sampler.get_stroke());
		sampler.clear();
	}

	whait_for_event = !sampler.active_state();
}

void StrokeApp::MainDrawTick() {
	window.col_clear = layer.canvas_color;
	mat4f cammat = (cam.projmat() * cam.viewmat()).transpose();
	sampler.draw(cammat);
	layer.draw(cammat);

	gui_draw();
}

void StrokeApp::camera_controller() {

	vec2f prevcur = window.prevcursor(1);
	vec2f cur = window.cursor(1);

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

	file.write<rgba>(&layer.canvas_color);
	file.write<camera>(&cam);

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

	file.read<rgba>(&layer.canvas_color);
	file.read<camera>(&cam);

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