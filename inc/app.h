
#pragma once

#include "gl.h"
#include "glutils.h"
#include "window.h"
#include "fbuffer.h"

#include "filesystem.h"

#include "strokes.h"

#include "ImGuiClass.h"

class StrokeApp : public ImGui::CompleteApp {

	camera cam;
	drawlayer layer;
	inputsmpler sampler;

	public:

	StrokeApp(vec2f size = vec2f(1400, 800));
	~StrokeApp();

	private:

	void save(File& file);
	void load(File& file);

	void MainProcTick();
	void MainDrawTick();

	void camera_controller();


	// FIXEME: Clean Up
	private:

	bool pushed(const rectf& rec);

	rgba uicol = rgba(0.14, 0.14, 0.15, 0.97);
	rgba fillcol = rgba(0.56, 0.56, 0.56, 1);

	halnf roundness = 10.f;

	rectf tool_bar_rect;
	vec2f popup_size = vec2f(200, 250);
	bool tollbar_popup = false;

	bool gui_active = false;
	bool item_howered = false;

	bool show_properties = false;

	void gui_draw();

	bool button(rectf& rect);
	bool pupup(rectf rect, float safe_padding);
	void FloatSlider(rectf rect, float& val, float min, float max);
	void ColorPicker(rectf rect, rgba& col);

	void draw_toolbar(rectf rect);
	void draw_properties();

	void draw_brush_properties(rectf rect);
	void draw_butt_undo_redo(rectf rect, bool flip = false);
};