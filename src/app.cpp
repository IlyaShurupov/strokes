
#include "app.h"

StrokeApp::StrokeApp(vec2f size) : ImGui::CompleteApp(size, ogl::window::FULL_SCREEN, "rsc/style"), objects_gui("data.strokes") {
	window.col_clear = rgba(0.2, 0.2, 0.23, 1);
	main_window = false;
	window.minsize.assign(400, 400);

	Object* new_scratch = NULL;
	alni dict_idx = objects_gui.root->items.Presents("Scratch");
	if (dict_idx == -1) {
		new_scratch = NDO->create("strokes");
		objects_gui.root->items.Put("Scratch", new_scratch);
	} else {
		new_scratch = objects_gui.root->items[dict_idx];
	}
	objects_gui.cd(new_scratch, "Scratch");
}

void StrokeApp::MainProcTick() {

	if (!objects_gui.active || objects_gui.active->type->name != "strokes") {
		project = NULL;
		window.col_clear = rgba(0.22, 0.22, 0.25, 1);
		return;
	}

	project = &NDO_CAST(StrokesObject, objects_gui.active)->project;

	if (gui_is_active) {
		return;
	}

	if (!project->active_layer) {
		return;
	}

	camera_controller();

	halnf pen_pressure = ImGui::BezierValue(window.pen_pressure(), tablet_input_formater);
	project->sampler.sample(&project->active_layer->strokes, &project->active_layer->strokes_undo, window.cursor(1), pen_pressure, &project->cam);

	if (!project->sampler.active_state() && project->sampler.has_input()) {
		stroke& str = project->sampler.get_stroke();

		str.denoise_positions(project->denoise_passes);
		str.denoise_thickness(project->denoise_passes_thikness);

		if (project->auto_reduction) {
			str.reduce_nof_points(project->pass_factor);
		}

		project->active_layer->add_stroke(project->sampler.get_stroke());
		project->sampler.clear();
	}

	whait_for_event = !project->sampler.active_state();
}

void StrokeApp::MainDrawTick() {
	gui_draw();

	if (project) {
		mat4f cammat = (project->cam.projmat() * project->cam.viewmat()).transpose();
		project->sampler.draw(cammat);

		for (alni idx = project->layers.Len() - 1; idx > -1; idx--) {
			if (!project->layers[idx]->hiden) {
				project->layers[idx]->draw(cammat);
			}
		}

		window.col_clear = project->canvas_color;
	}
}

void StrokeApp::camera_controller() {

	vec2f prevcur = window.prevcursor(1);
	vec2f cur = window.cursor(1);

	vec2f delta = prevcur - cur;
	vec3f target = project->cam.get_target();

	if (glfwGetMouseButton(window.geth(), GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
		project->cam.rotate(delta.x * 5, delta.y * 5);
	}

	if (glfwGetMouseButton(window.geth(), GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
		project->cam.zoom((prevcur.y + 1.f) / (cur.y + 1.f));
	}

	if (glfwGetKey(window.geth(), GLFW_KEY_SPACE) == GLFW_PRESS) {
		project->cam.move(cur, prevcur);
	}

	project->cam.set_ratio(window.aspect_ratio());
}

StrokeApp::~StrokeApp() {}