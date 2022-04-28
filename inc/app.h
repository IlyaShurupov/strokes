
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
	bool gui_is_active = true;
	rectf tool_bar_rect;
	vec2f popup_size = vec2f(200, 250);
	bool tollbar_popup = false;

	// application specific
	camera cam;
	drawlayer layer;
	inputsmpler sampler;

	public:

	StrokeApp(vec2f size = vec2f(1400, 1000));
	alni run();
	~StrokeApp();

	private:
	
	void save(File& file);
	void load(File& file);

	void camera_controller();
	void proc_inputs();
	void send_output();
	void draw_brush_properties(rectf rect);
	void draw_toolbar(rectf rect);
	void draw_ui();
};