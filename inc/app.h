
#pragma once

#include "gl.h"
#include "glutils.h"
#include "window.h"
#include "fbuffer.h"

#include "filesystem.h"

#include "strokes.h"

#include "ImGuiClass.h"
#include "ImGuiObjectEditor.h"

#include "primitives/primitives.h"
#include "strokesobject.h"

class StrokeApp : public ImGui::CompleteApp {

	ImGui::ImGuiObjectEditor objects_gui;
	strokes_project* project = NULL;

	tp::halnf tablet_input_formater[5] = {0.5f, 0.f, 1.f, .5f};

	public:

	StrokeApp(tp::vec2f size = tp::vec2f(1400, 800));
	~StrokeApp();

	private:

	void MainProcTick();
	void MainDrawTick();

	void camera_controller();

	private:

	void gui_draw();
	void draw_explorer();
	void draw_toolbar(tp::rectf rect);
	void draw_brush_properties(tp::rectf rect);
};