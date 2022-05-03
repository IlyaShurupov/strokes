
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

	public:

	StrokeApp(vec2f size = vec2f(1400, 800));
	~StrokeApp();

	private:

	void MainProcTick();
	void MainDrawTick();

	void camera_controller();

	private:

	rgba uicol = rgba(0.37, 0.37, 0.41, 0.97);
	rgba fillcol = rgba(0.75, 0.75, 0.75, 1);

	bool show_properties = false;
	bool show_explorer = false;

	void gui_draw();
	void draw_explorer();
	void draw_toolbar(rectf rect);
	void draw_properties();
	void draw_brush_properties(rectf rect);
};