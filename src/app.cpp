
#include "app.h"

#include "imgui.h"
#include "imgui_utils.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

time_ms StrokeApp::dur(float fps) {
	time_ms out = 1000.f / fps;
	return out;
}

StrokeApp::StrokeApp(vec2f size) : ogl(), window(size), fbo(size, rgba(0)), gui(&window) {

	File db("data.strokes", osfile_openflags::LOAD);
	load(db);
	db.close();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void) io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window.geth(), true);
	ImGui_ImplOpenGL3_Init("#version 130");

	//ImPlot::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImFont* font1 = io.Fonts->AddFontFromFileTTF(get_font_path(), 18.0f);

	init_notify();
	apply_style();
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
		cam.zoom((prevcur.y + 0.5f) / (cur.y + 0.5f));
	}

	if (glfwGetKey(window.geth(), GLFW_KEY_SPACE) == GLFW_PRESS) {
		cam.move(cur, prevcur);
	}

	cam.set_ratio(window.aspect_ratio());
	prevcur = cur;
}

void StrokeApp::proc_inputs() {

	if (!can_draw) {
		return;
	}

	camera_controller();
	sampler.sample(&layer.strokes, &layer.strokes_undo, window.cursor(true), window.pen_pressure(), &cam);

	whait_for_ev = sampler.state == inputsmpler::pstate::NONE;

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
		sampler.draw(cam.projmat(), cam.viewmat());
		layer.draw(cam.projmat(), cam.viewmat());

		draw_ui();

		if (show_debug) {
			render_debug_ui();
		}

	} window.end_draw(/*whait_for_ev*/);
}

void StrokeApp::mainloop() {
	timer tm_frame_time(whait_for_ev ? dur(idle_device_fps) : dur(max_device_fps));


	timer tm;

	timer tm_draw_time;
	send_output();
	draw_time = tm_draw_time.past();


	int frames_precessed = 0;
	int target_input_ratio = whait_for_ev ? idle_input_ratio : max_input_ratio;
	while ((tm.past() < dur(max_device_fps) || frames_precessed == 0) && (frames_precessed < target_input_ratio)) {
		proc_inputs();
		glfwPollEvents();
		frames_precessed++;
	}
	input_vs_output = frames_precessed;

	frame_time = tm_frame_time.past();
	tm_frame_time.wait_out();
}

void StrokeApp::draw_brush_properties(rectf rect) {
	float slider_size = 40;
	float picker_size = 180;

	rectf slider_rec = rectf(rect.x, rect.w + rect.y - slider_size, rect.z, slider_size);
	if (sampler.eraser) {
		gui.FloatSlider(slider_rec, sampler.eraser_size, 0.01, 0.3);
		popup_size.y = slider_size;
	} else {
		gui.FloatSlider(slider_rec, sampler.thickness, 0.01, 0.25);
		halnf size = MIN(rect.z, rect.w - (slider_size + 10));
		gui.ColorPicker(rectf(rect.x + (rect.z - size) / 2, rect.y, size, size), sampler.stroke_col);

		popup_size.y = picker_size + slider_size + 10;
	}

	popup_size.x = picker_size;
}

void StrokeApp::draw_toolbar(rectf rect) {

	rectf(*get_rect)(rectf & in) = [](rectf& in) {
		static int idx = 0;
		float butns = 6;
		float item_size = in.z / butns;
		float padding = item_size / 10;
		item_size = item_size - padding;

		rectf out = rectf(in.x + idx * (item_size + padding * 2), in.y, item_size, in.w);

		if (idx == 5) {
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
	if (gui.button(get_rect(rect), NULL, "../rsc/icons/Debug.png")) {
		show_debug = !show_debug;
	}
	if (gui.button(get_rect(rect), NULL, "../rsc/icons/Quit.png")) {
		quit = 1;
	}
}

void StrokeApp::draw_ui() {
	draw_toolbar(tool_bar_rect);

	halnf cur_scale = (sampler.eraser ? sampler.eraser_size : sampler.thickness) * window.size.x;
	rectf cur_rect = rectf(window.cursor().x - cur_scale / 2, window.cursor().y - cur_scale / 2, cur_scale, cur_scale);

	window.set_viewport(cur_rect);
	draw_texture(0, get_tex("../rsc/icons/EraserCursor.png"));

	can_draw = !gui.gui_active;
	gui.gui_active = false;
}

void StrokeApp::render_debug_ui() {

	can_draw = !ImGui::GetIO().WantCaptureMouse;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin(" ");

	ImGuiColorEditFlags_ flg;
	flg = ImGuiColorEditFlags_(ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_AlphaBar);

	ImGui::Text("\n\nInspired by kuko Developed by dudka <3\n\n");

	ImGui::ColorEdit4("Background Color", &window.col_clear.r, flg);
	ImGui::ColorEdit4("Stroke Color", &sampler.stroke_col.rgbs.r, flg);
	ImGui::SliderFloat("Stroke Thikness", &sampler.thickness, 0.009, 0.1);
	ImGui::SliderFloat("Sampler Precision", &sampler.precision, 0.01, 0.1);


	ImGui::Separator();
	ImGui::SliderInt("max_input_ratio", &max_input_ratio, 1, 200);
	ImGui::SliderInt("idle_input_ratio", &idle_input_ratio, 1, 200);
	ImGui::SliderInt("max_device_fps", &max_device_fps, 10, 600);
	ImGui::SliderInt("idle_device_fps", &idle_device_fps, 10, 600);
	ImGui::Separator();
	ImGui::Text("FPS (screen update) %i", fps);
	ImGui::Text("PPS %f", fps * input_vs_output);
	ImGui::Text("Idle State %i", whait_for_ev);
	ImGui::Separator();
	ImGui::Text("Frame Time %f ms", frame_time);
	ImGui::Text("Draw Time %f ms", draw_time);
	ImGui::Text("Proc Time %f ms", frame_time - draw_time);
	ImGui::Separator();
	ImGui::SliderFloat("toolbar pos x", &tool_bar_rect.x, 0.f, 1000);
	ImGui::SliderFloat("toolbar pos y", &tool_bar_rect.y, 0.f, 1000);
	ImGui::SliderFloat("toolbar size x", &tool_bar_rect.z, 0.f, 1000);
	ImGui::SliderFloat("toolbar size y", &tool_bar_rect.w, 0.f, 1000);


	ImGui::End();

	render_notify();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool StrokeApp::IsRunnign() {
	return !quit && !window.CloseSignal();
}

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
		layer.add_stroke(stroke());
		stroke& str = layer.strokes.Last()->data;

		alni p_len; file.read<alni>(&p_len);
		rgba color; file.read<rgba>(&color);

		str.points.Reserve(p_len);

		for (auto piter : str.points) {
			file.read<stroke_point>(&piter.data());
		}

		str.gen_mesh();
		str.mesh.color = color;
	}
}

StrokeApp::~StrokeApp() {
	// Cleanup
	//ImPlot::DestroyContext();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	File db;
	db.open("data.strokes", osfile_openflags::SAVE);
	save(db);
	db.close();
}
