
#include "app.h"

#include "glutils.h"

#include "shader.h"
#include "strings.h"

#include "imgui_internal.h"

using namespace tp;

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
	const halnf nbuttons = project ? 6 : 1;
	halnf buttsize = clamp(rec.z / nbuttons, 5.f, 50.f) + 30;
	vec2f buttrec = {buttsize - 20, buttsize - 50};

	vec2f midpoint = (rec.pos + rec.size_vecw()) / 2.f;
	rec.size = {buttsize * nbuttons + 20, buttsize - 30};
	rec.pos = midpoint - (rec.size / 2.f);

	ImGui::SetNextWindowPos(ImVec2(rec.x, window.size.y - rec.y));
	ImGui::SetNextWindowSize(ImVec2(rec.z, rec.w));
	ImGui::Begin("ToolBar", 0, ImGui::frame_window);

	vec2f popup_size = vec2f(200, 200);

	auto popup = ImGui::ButtonHoverPopupBegin("Explore", buttrec, popup_size);
	if (popup) { draw_explorer(); }
	ImGui::HoverPopupEnd(popup);

	if (!project) {
		ImGui::End();
		return;
	}

	ImGui::SameLine();
	popup = ImGui::ButtonHoverPopupBegin("Layers", buttrec, popup_size);
	if (popup) {
		if (ImGui::Button("Add New", ImVec2(150, 30))) {
			project->layers.pushBack(new drawlayer());
		}

		ImGui::Separator();

		for (auto lay : project->layers) {
			ImGui::PushID((int) lay.Idx());
			
			bool higlight = lay.Data() == project->active_layer;
			if (higlight) {
				ImGui::PushStyleColor(ImGuiCol_Button, GImGui->Style.Colors[ImGuiCol_ButtonActive]);
			}

			if (ImGui::Button(lay->name.cstr(), ImVec2(100, 30))) {
				project->active_layer = lay.Data();
			}

			if (higlight) {
				ImGui::PopStyleColor();
			}


			if (ImGui::BeginPopupContextItem(lay->name.cstr(), ImGuiPopupFlags_MouseButtonRight)) {
				static char name[150] = {"asdas"};
				if (ImGui::InputTextEx(" ", "new name", name, 150, {150 , 30}, ImGuiInputTextFlags_EnterReturnsTrue)) {
					lay->name = name;
					lay->name.capture();
				}
				if (ImGui::Button("Delete", ImVec2(150, 30))) {
					project->active_layer = lay.node()->prev ? lay.node()->prev->data : NULL;
					delete lay.node()->data;
					project->layers.delNode(lay.node());
					ImGui::EndPopup();
					ImGui::PopID();
					break;
				}
				if (lay.node()->prev && ImGui::Button("Move Up", ImVec2(150, 30))) {
					swap(lay.node()->prev->data, lay.Data());
				}
				if (lay.node()->next && ImGui::Button("Move Down", ImVec2(150, 30))) {
					swap(lay.node()->next->data, lay.Data());
				}
				ImGui::EndPopup();
			}

			ImGui::SameLine();
			bool visable = !lay->hiden;
			ImGui::Checkbox("", &visable);
			lay->hiden = !visable;

			ImGui::PopID();
		}
	}
	HoverPopupEnd(popup);

	if (!project->active_layer) {
		ImGui::End();
		return;
	}

	ImGui::SameLine();
	popup = ImGui::ButtonHoverPopupBegin("Hist", buttrec, vec2f(100));
	if (popup) {
		if (ImGui::Button("Undo", ImVec2(100, 30))) {
			project->active_layer->undo();
		}
		if (ImGui::Button("Redo", ImVec2(100, 30))) {
			project->active_layer->redo();
		}
		if (ImGui::Button("Clear History", ImVec2(100, 30))) {
			project->active_layer->clear_history();
		}
	}
	HoverPopupEnd(popup);

	ImGui::SameLine();
	popup = ImGui::ButtonHoverPopupBegin("Tool", buttrec, popup_size);
	if (popup) { draw_brush_properties(rectf(0, 0, 300, 300)); }
	ImGui::HoverPopupEnd(popup);


	ImGui::SameLine();
	popup = ImGui::ButtonHoverPopupBegin("Proj", buttrec, popup_size);
	if (popup) {
		int flags = ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs;
		ImGui::ColorPicker4("Background Color", &project->canvas_color.r, flags);

		halni width = ImGui::GetWindowContentRegionWidth();
		if (ImGui::Button("Clear All Strokes", ImVec2(width, 30))) {
			project->active_layer->clear_canvas();
		}

		if (ImGui::Button("Reduce Size", ImVec2(width, 30))) {
			alni length = project->save_size() / (1024);
			project->active_layer->reduce_size(project->pass_factor);
			alni length_after = project->save_size() / (1024);
			ImGui::Notify(sfmt("Size Redused from %i to %i kb", length, length_after).cstr());
		}

		ImGui::SetNextItemWidth(width / 2);
		ImGui::SliderFloat("Pass Facor", &project->pass_factor, 0, 0.01);
	}
	HoverPopupEnd(popup);

	ImGui::SameLine();
	popup = ImGui::ButtonHoverPopupBegin("Cam", buttrec, vec2f(100));
	if (popup) {
		if (ImGui::Button("Reset", ImVec2(100, 30))) {
			project->cam.lookat({0, 0, 0}, {100, 0, 0}, {0, 0, 1});
		}
		halnf fov = project->cam.get_fov();
		ImGui::Text("Field Of View");
		ImGui::SetNextItemWidth(110);
		ImGui::SliderFloat(" ", &fov, 0.1, PI - 0.1);
		project->cam.set_fov(fov);
	}
	HoverPopupEnd(popup);

	ImGui::End();
}

void StrokeApp::gui_draw() {

	halnf toolbar_size = 290;
	draw_toolbar(rectf(window.size.x / 2.f - toolbar_size / 2.f, window.size.y, toolbar_size, 60));

	if (!project) {
		DrawTextR(rectf(window.size / 2.2, 0), "Select Strokes Object", rgba(0.75, 0.75, 0.75, 1));

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