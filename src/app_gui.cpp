
#include "app.h"

#include "glutils.h"

#include "shader.h"

void StrokeApp::draw_properties() {

	halni butt_size = 50.f;
	halnf prop_size = 450;
	rectf rec = rectf(window.size.x - (show_properties ? prop_size : 50), window.size.y, butt_size, butt_size);

	ImGui::SetNextWindowPos(ImVec2(rec.x, window.size.y - rec.y));
	ImGui::SetNextWindowSize(ImVec2(rec.z, rec.w));
	ImGui::Begin("Properties button", 0, ImGui::frame_window);
	const char* label = show_properties ? " > " : " < ";
	if (ImGui::Button(label)) {
		show_properties = !show_properties;
	}
	ImGui::End();

	if (!show_properties) return;

	using namespace ImGui;

	{
		ImGui::SetNextWindowPos(ImVec2(rec.pos.x + rec.z / 2.f + 30, 10));
		ImGui::SetNextWindowSize(ImVec2(prop_size - 60, window.size.y - 20));

		//ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::Begin("Properties", 0, window_flags);

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (BeginMenuBar()) {
			Text("Properties");
			EndMenuBar();
		}
	}

	Begin("View");
	if (Button("Reset Camera")) {
		project->cam.lookat({0, 0, 0}, {100, 0, 0}, {0, 0, 1});
	}
	halnf fov = project->cam.get_fov();
	SliderFloat("Field Of View", &fov, 0.1, PI - 0.1);
	project->cam.set_fov(fov);
	End();

	Begin("Canvas");

	ImGui::Text("Background Color");

	auto popup = HoverPopupBegin("sa");
	if (popup) {
		int flags = ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs;
		ColorPicker4("Back Color", &project->layer.canvas_color.r, flags);
	}
	HoverPopupEnd(popup);

	End();

	Begin("Strokes");
	SliderFloat("Input Precision", &project->sampler.screen_precision, 0, 0.01);
	if (SubMenuBegin("Denoise Passes", 1)) {
		SliderInt("Position", &project->denoise_passes, 0, 10);
		SliderInt("Thickness", &project->denoise_passes_thikness, 0, 10);
		SubMenuEnd(1);
	}
	Text("Reduction");
	End();

	if (Begin("UI")) {
		Text("Todo");
		if (0) {
			ColorPicker4("Ui Color", &uicol.r);
			ColorPicker4("Accent Color", &fillcol.r);

			StyleEditor();
			apply_style();
		}
	}
	End();

	End();
}

void StrokeApp::draw_explorer() {
	halni butt_size = 50;
	halnf prop_size = 400;
	rectf rec = rectf(0 + (show_explorer ? prop_size - 40 : 0), window.size.y, butt_size, butt_size);

	ImGui::SetNextWindowPos(ImVec2(rec.x, window.size.y - rec.y));
	ImGui::SetNextWindowSize(ImVec2(rec.z, rec.w));
	ImGui::Begin("Explorer button", 0, ImGui::frame_window);

	const char* label = show_explorer ? " < " : " > ";
	if (ImGui::Button(label)) {
		show_explorer = !show_explorer;
	}
	ImGui::End();

	if (!show_explorer) return;

	using namespace ImGui;

	{
		SetNextWindowPos(ImVec2(rec.pos.x + rec.z / 2.f + 30 - prop_size, 10));
		SetNextWindowSize(ImVec2(prop_size - 60, window.size.y - 20));

		//ImGui::SetNextWindowViewport(viewport->ID);
		PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		Begin("Objects Explorer", 0, window_flags);

		PopStyleVar();
		PopStyleVar();

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (BeginMenuBar()) {
			Text("Object Explorer");
			EndMenuBar();
		}
	}

	End();

	objects_gui.Draw();
}

void StrokeApp::draw_brush_properties(rectf rect) {
	ImGui::SetNextItemWidth(rect.w);
	if (project->sampler.eraser) {
		ImGui::SliderFloat(" ", &project->sampler.eraser_size, 0.001f, 0.2f);
	} else {
		ImGui::SliderFloat(" ", &project->sampler.screen_thikness, 0.001f, 0.2f);

		int flags = ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs;
		ImGui::ColorPicker4("Color", &project->sampler.stroke_col.r, flags);
	}
}

void StrokeApp::draw_toolbar(rectf rec) {

	ImGui::SetNextWindowPos(ImVec2(rec.x, window.size.y - rec.y));
	ImGui::SetNextWindowSize(ImVec2(rec.z, rec.w));

	ImGui::Begin("ToolBar", 0, ImGui::frame_window);

	if (ImGui::Button("Undo")) {
		project->layer.undo();
	} ImGui::SameLine();

	if (ImGui::Button("Redo")) {
		project->layer.redo();
	} ImGui::SameLine();

	const char* tool_lable = project->sampler.eraser ? "Brush" : "Eraser";
	if (ImGui::Button(tool_lable)) {
		project->sampler.eraser = !project->sampler.eraser;
	} ImGui::SameLine();

	const char* properties_lable = project->sampler.eraser ? "Size" : "Color";
	if (ImGui::Button(properties_lable)) {
		//project->sampler.eraser = !project->sampler.eraser;
	} ImGui::SameLine();

	auto popup = ImGui::HoverPopupBegin("ToolProperties", vec2f(150, 150), vec2f(rec.x + rec.w / 2 - 30, window.size.y - rec.y + rec.w - 10));
	if (popup) { draw_brush_properties(rectf(0, 0, 300, 300)); }
	ImGui::HoverPopupEnd(popup);

	if (ImGui::Button("Clear")) {
		alni len = project->layer.strokes.Len();
		for (alni idx = 0; idx < len; idx++) {
			project->layer.undo();
		}
	} ImGui::SameLine();

	if (ImGui::Button("Exit")) {
		window.post_quit_event();
	}

	ImGui::End();
}

void StrokeApp::gui_draw() {
	draw_toolbar(rectf(window.size.x / 2.f - 185, window.size.y, 370, 60));
	draw_properties();

	halnf cur_scale = (project->sampler.eraser ? project->sampler.eraser_size : project->sampler.screen_thikness / 2.f) * window.size.x / 2.f;
	DrawCircle(window.cursor(), cur_scale, rgba(0, 0, 0, 0.7), 4.f);
	DrawCircle(window.cursor(), cur_scale, project->sampler.stroke_col, 2.f);
}