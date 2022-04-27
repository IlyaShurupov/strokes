
#pragma once

#include "gl.h"
#include "glutils.h"
#include "window.h"
#include "fbuffer.h"

#include "filesystem.h"

#include "strokes.h"
#include "GUI.h"

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

	rectf tool_bar_rect = rectf(window.size.x / 2.f - 350 / 2.f, window.size.y - 70, 350, 50);
	vec2f popup_size = vec2f(200, 250);
	bool tollbar_popup = false;

	public:

	int fps;

	StrokeApp(vec2f size);
	void mainloop();
	bool IsRunnign();
	void save(File& file);
	void load(File& file);
	~StrokeApp();

	private:

	time_ms dur(float fps);

	void camera_controller();
	void proc_inputs();
	void send_output();
	void draw_brush_properties(rectf rect);
	void draw_toolbar(rectf rect);
	void draw_ui();
	void render_debug_ui();
};