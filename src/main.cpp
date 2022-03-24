

#include "gl.h"
#include "window.h"
#include "fbuffer.h"
#include "glutils.h"

#include "strokes.h"
#include "controller.h"

#include "GUI.h"

#include "common.h"
#include "osystem.h"

#include "imgui.h"
#include "imgui_utils.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct StrokeApp_SaveHeader {
	char name[10] = { 0 };
	char version[10] = { 0 };
	alni nstrokes = 0;

	StrokeApp_SaveHeader(alni nstrokes) {
		memcp(&name, "strokes", slen("strokes") + 1);
		memcp(&version, "0", slen("0") + 1);
		this->nstrokes = nstrokes;
	}

	StrokeApp_SaveHeader() {}
};

class StrokeApp {

	ogl::opengl ogl;
	ogl::window window;
	ogl::fbuffer fbo;

	GuiState gui;
	camera cam;
	drawlayer layer;
	inputsmpler sampler;

	bool quit = false;
	bool show_debug = false;
	bool can_draw = true;

	float frame_time;
	float draw_time;
	float input_vs_output;

	int max_input_ratio = 200.f;
	int idle_input_ratio = 2.f;
	int max_device_fps = 100.f;
	int idle_device_fps = 40.f;
	bool whait_for_ev = false;


	time_ms dur(float fps) {
		time_ms out = 1000.f / fps;
		return out;
	}

	vec4 tool_bar_rect = vec4(740, 980, 350, 50);
	vec2 popup_size = vec2(200, 250);
	bool tollbar_popup = false;
public:
	
	int fps;

	StrokeApp(vec2 size) : ogl(), window(size), fbo(size, vec4(0)), gui(&window) {

		osfile db("data.strokes", osfile_openflags::LOAD);
		load(db);
		db.close();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
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

	void proc_inputs() {

		if (!can_draw) {
			return;
		}

		camera_controller(window.geth(), &cam);
		sampler.sample(&layer.strokes, &layer.strokes_undo, window.cursor(true), window.pen_pressure(), &cam);
		
		whait_for_ev = sampler.state == inputsmpler::pstate::NONE;

		if (!sampler.active_state() && sampler.has_input()) {
			layer.add_stroke(sampler.get_stroke());
			sampler.clear();
		}
	}

	void send_output() {

		window.begin_draw(); {
			window.clear();

			sampler.draw(cam.projmat(), cam.viewmat());
			layer.draw(cam.projmat(), cam.viewmat());

			draw_ui();

			if (show_debug) {
				render_debug_ui();
			}

		} window.end_draw(/*whait_for_ev*/);
	}

	void mainloop() {
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

	void draw_brush_properties(vec4 rect) {
		float slider_size = 40;
		float picker_size = 180;

		vec4 slider_rec = vec4(rect.x, rect.w + rect.y - slider_size, rect.z, slider_size);
		if (sampler.eraser) {
			gui.FloatSlider(slider_rec, sampler.eraser_size, 0.01, 0.3);
			popup_size.y = slider_size;
		}
		else {
			gui.FloatSlider(slider_rec, sampler.thickness, 0.01, 0.25);
			float size = glm::min(rect.z, rect.w - (slider_size + 10));
			gui.ColorPicker(vec4(rect.x + (rect.z- size) / 2, rect.y, size, size), sampler.stroke_col);

			popup_size.y = picker_size + slider_size + 10;
		}

		popup_size.x = picker_size;
	}

	void draw_toolbar(vec4 rect) {

		vec4(*get_rect)(vec4 & in) = [](vec4& in) { 
			static int idx = 0;
			float butns = 6;
			float item_size = in.z / butns;
			float padding = item_size / 10;
			item_size = item_size - padding;
			
			vec4 out = vec4(in.x + idx * (item_size + padding * 2), in.y, item_size, in.w);
			
			if (idx == 5) {
				idx = 0;
			}
			else {
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
			vec4 butrec = get_rect(rect);
			if (gui.button(butrec, NULL, sampler.eraser? "../rsc/icons/Eraser.png" : "../rsc/icons/Pen.png")) {
				sampler.eraser = !sampler.eraser;
			}
			
			bool activator_howered = gui.item_howered;
			if (activator_howered) {
				tollbar_popup = true;
			}

			if (tollbar_popup) {
				vec4 poup_rec = vec4(butrec.x + butrec.z / 2 - popup_size.x / 2, butrec.y - 25 - popup_size.y, popup_size.x, popup_size.y);

				bool should_close = !gui.pupup(poup_rec, 40);

				if (should_close && !activator_howered) {
					tollbar_popup = false;
				}
				else {
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

	void draw_ui() {
		draw_toolbar(tool_bar_rect);

		float cur_scale = (sampler.eraser ? sampler.eraser_size : sampler.thickness) * window.size.x;
		vec4 size = vec4(window.cursor().x - cur_scale / 2, window.cursor().y - cur_scale / 2, cur_scale, cur_scale);
		GLuint tex = get_tex("../rsc/icons/EraserCursor.png");
		draw_texture(0, tex, vec4(0, 0, window.size.x, window.size.y), size);

		can_draw = !gui.gui_active;
		gui.gui_active = false;
	}

	void render_debug_ui() {

		can_draw = !ImGui::GetIO().WantCaptureMouse;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin(" ");

		ImGuiColorEditFlags_ flg;
		flg = ImGuiColorEditFlags_(ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_AlphaBar);

		ImGui::Text("\n\nInspired by kuko Developed by dudka <3\n\n");

		ImGui::ColorEdit4("Background Color", &window.col_clear.x, flg);
		ImGui::ColorEdit4("Stroke Color", &sampler.stroke_col.x, flg);
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

	bool IsRunnign() {
		return !quit && window.CloseSignal();
	}

	void save(osfile& file) {
		StrokeApp_SaveHeader head(layer.strokes.Len());
		file.write<StrokeApp_SaveHeader>(&head);

		for (auto stiter : layer.strokes) {
			stroke* str = &stiter.Data();
			file.write<alni>(&str->points.length);
			file.write<vec4>(&str->mesh.color);

			for (auto piter : str->points) {
				file.write<stroke_point>(&piter.data());
			}
		}
	}

	void load(osfile& file) {
		StrokeApp_SaveHeader head;
		file.read<StrokeApp_SaveHeader>(&head);
		if (!memequal(head.name, "strokes", slen("strokes"))) {
			return;
		}

		for (alni str_idx = 0; str_idx < head.nstrokes; str_idx++) {
			layer.add_stroke(stroke());
			stroke& str = layer.strokes.Last()->data;

			alni p_len; file.read<alni>(&p_len);
			vec4 color; file.read<vec4>(&color);

			str.points.Reserve(p_len);

			for (auto piter : str.points) {
				file.read<stroke_point>(&piter.data());
			}

			str.gen_mesh();
			str.mesh.color = color;
		}
	}

	~StrokeApp() {
		// Cleanup
		//ImPlot::DestroyContext();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		osfile db;
		db.open("data.strokes", osfile_openflags::SAVE);
		save(db);
		db.close();
	}
};


int main() {

	StrokeApp app(vec2(1920, 1080));

	frame_counter fc;

	do {
		fc.log(app.fps);
		app.mainloop();
	} while (app.IsRunnign());

	return 0;
}

int WinMain() {
	return main();
}