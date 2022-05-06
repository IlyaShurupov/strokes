
#include "app.h"

#include "glutils.h"

#include "shader.h"
#include "strings.h"

void StrokeApp::draw_explorer() {
	using namespace ImGui;
	objects_gui.oexplorer(200);
	if (SubMenuBegin("Object Info", 1)) {
		objects_gui.oproperties(objects_gui.active->type);
		SubMenuEnd(1);
	}
}

void StrokeApp::draw_brush_properties(rectf rect) {
	using namespace ImGui;
	if (ImGui::Button(project->sampler.eraser ? "Eraser" : "Pencil")) {
		project->sampler.eraser = !project->sampler.eraser;
	} ImGui::SameLine();

	SetNextItemWidth(rect.z / 2.f);
	//ImGui::SetNextItemWidth(rect.w);
	if (project->sampler.eraser) {
		ImGui::SliderFloat(" ", &project->sampler.eraser_size, 0.001f, 0.2f);
	} else {
		ImGui::SliderFloat(" ", &project->sampler.screen_thikness, 0.001f, 0.2f);

		SetNextItemWidth(rect.z / 1.5f);
		int flags = ImGuiColorEditFlags_NoLabel |
			ImGuiColorEditFlags_NoSmallPreview |
			ImGuiColorEditFlags_NoSidePreview |
			ImGuiColorEditFlags_NoInputs;
		ImGui::ColorPicker4("Color", &project->sampler.stroke_col.r, flags);

		PushItemWidth(rect.z / 4.f);
		if (SubMenuBegin("Sampler Settings", 1)) {

			if (SubMenuBegin("Denoising Passes", 2)) {
				SliderInt("Position", &project->denoise_passes, 0, 10);
				SliderInt("Thickness", &project->denoise_passes_thikness, 0, 10);
				SubMenuEnd(2);
			}

			if (SubMenuBegin("Size Reduction", 2)) {
				SliderFloat("Input Precision", &project->sampler.screen_precision, 0, 0.005);
				Checkbox("Auto Apply", &project->auto_reduction);
				SubMenuEnd(2);
			}

			if (SubMenuBegin("Pen Pressure", 2)) {
				ImGui::Bezier("easeOutSine", tablet_input_formater);
				SubMenuEnd(2);
			}

			SubMenuEnd(1);
		}
		PopItemWidth();
	}
	
	
}

void StrokeApp::draw_toolbar(rectf rec) {
	ImGui::SetNextWindowPos(ImVec2(rec.x, window.size.y - rec.y));
	ImGui::SetNextWindowSize(ImVec2(rec.z, rec.w));
	ImGui::Begin("ToolBar", 0, ImGui::frame_window);

	if (!project) {
		//ImGui::Indent(rec.z / 2.f - 50);
	}

	ImGui::Button("Explorer"); ImGui::SameLine();
	auto popup = ImGui::HoverPopupBegin("Explorer", vec2f(150, 150),
		vec2f(rec.x + rec.w / 2 - 100, window.size.y - rec.y + rec.w - 10));
	if (popup) { draw_explorer(); }
	ImGui::HoverPopupEnd(popup);


	if (project) {

		ImGui::Button("History"); ImGui::SameLine();
		auto undo_popup = ImGui::HoverPopupBegin("History",
			vec2f(150, 150), vec2f(rec.x + rec.w / 2 + 30, window.size.y - rec.y + rec.w - 10));
		if (undo_popup) {
			if (ImGui::Button("       Undo       "))
				project->layer.undo();
			if (ImGui::Button("       Redo       "))
				project->layer.redo();
			if (ImGui::Button("Clear History"))
				project->layer.clear_history();
		}
		ImGui::HoverPopupEnd(undo_popup);

		ImGui::Button("Tool"); ImGui::SameLine();
		auto popup = ImGui::HoverPopupBegin("ToolProperties",
			vec2f(150, 150), vec2f(rec.x + rec.w / 2 + 70, window.size.y - rec.y + rec.w - 10));
		if (popup) {
			draw_brush_properties(rectf(0, 0, 300, 300));
		}
		ImGui::HoverPopupEnd(popup);

		ImGui::Button("Project"); ImGui::SameLine();
		auto canvas_popup = ImGui::HoverPopupBegin("Project",
			vec2f(150, 150), vec2f(rec.x + rec.w / 2 + 150, window.size.y - rec.y + rec.w - 10));
		if (canvas_popup) {
			int flags = ImGuiColorEditFlags_NoLabel |
				ImGuiColorEditFlags_NoSmallPreview |
				ImGuiColorEditFlags_NoSidePreview |
				ImGuiColorEditFlags_NoInputs;

			ImGui::ColorPicker4("Background Color", &project->layer.canvas_color.r, flags);

			halni width = ImGui::GetWindowContentRegionWidth();
			if (ImGui::Button("Clear All Strokes", ImVec2(width, 30)))
				project->layer.clear_canvas();

			if (ImGui::Button("Reduce Size", ImVec2(width, 30))) {
				alni length = project->save_size() / (1024);
				project->layer.reduce_size(project->pass_factor);
				alni length_after = project->save_size() / (1024);
				ImGui::Notify(sfmt("Size Redused from %i to %i kb", length, length_after).cstr());
			}
			ImGui::SetNextItemWidth(width / 2);
			ImGui::SliderFloat("Pass Facor", &project->pass_factor, 0, 0.01);
		}
		HoverPopupEnd(canvas_popup);

		ImGui::Button("Camera"); ImGui::SameLine();
		auto camera_popup = ImGui::HoverPopupBegin("Camera",
			vec2f(150, 150), vec2f(rec.x + rec.w / 2 + 200, window.size.y - rec.y + rec.w - 10));
		if (camera_popup) {
			if (ImGui::Button("          Reset         ")) {
				project->cam.lookat({0, 0, 0}, {100, 0, 0}, {0, 0, 1});
			}
			halnf fov = project->cam.get_fov();
			ImGui::Text("    Field Of View   ");
			ImGui::SetNextItemWidth(110);
			ImGui::SliderFloat(" ", &fov, 0.1, PI - 0.1);
			project->cam.set_fov(fov);
		}
		HoverPopupEnd(camera_popup);
	}

	if (ImGui::Button("Exit")) {
		window.post_quit_event();
	}

	ImGui::End();
}

void StrokeApp::gui_draw() {

	draw_toolbar(rectf(window.size.x / 2.f - 225, window.size.y, 440, 60));

	if (!project) {
		DrawTextR(rectf(window.size / 2, 0), "Select Strokes Object", rgba(0.75, 0.75, 0.75, 1));

	} else if (!gui_is_active || 1) {

		if (project->sampler.eraser) {
			halnf size = project->sampler.eraser_size * window.size.x / 2.f;
			DrawCircle(window.cursor(), size, rgba(0.9, 0.9, 0.9, 0.7), 2.f);
		} else {
			halnf size = project->sampler.screen_thikness / 2.f * window.size.x / 2.f;
			DrawCircle(window.cursor(), size, rgba(0, 0, 0, 0.7), 4.f);
			DrawCircle(window.cursor(), size, project->sampler.stroke_col, 2.f);
		}
	}
}