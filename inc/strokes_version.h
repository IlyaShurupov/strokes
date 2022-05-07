#pragma once 

#include "strokes.h"

struct ProjectInfo {
	char name[10] = {0};
	char version[10] = {0};
	
	void init(alni v) {
		string version_s(v);
		memcp(name, "strokes", slen("strokes"));
		memcp(version, version_s.cstr(), version_s.size());
	}
};

namespace StrokesVersion0 {

	alni save_size(strokes_project* proj) {
		alni out = 0;
		out += sizeof(camera);
		out += sizeof(rgba);

		out += sizeof(alni);
		for (auto layer : proj->layers) {
			for (auto stiter : layer->strokes) {
				stroke* str = &stiter.Data();
				
				out += sizeof(rgba);

				out += sizeof(alni);
				out += str->points.length * sizeof(stroke_point);
			}
		}
		return out;
	}

	void save(File& file, strokes_project* proj) {

		file.write<camera>(&proj->cam);
		file.write<rgba>(&proj->canvas_color);

		drawlayer base_layer;
		proj->append_layers(&base_layer);

		alni len = base_layer.strokes.Len();
		file.write<alni>(&len);
		for (auto stiter : base_layer.strokes) {
			stroke* str = &stiter.Data();

			file.write<alni>(&str->points.length);
			file.write<rgba>(&str->mesh.color);

			for (auto piter : str->points) {
				file.write<stroke_point>(&piter.data());
			}
		}
	}

	void load(File& file, strokes_project* proj) {
		file.read<camera>(&proj->cam);

		file.read<rgba>(&proj->canvas_color);

		drawlayer* base = proj->get_base_layer();

		alni len;
		file.read<alni>(&len);

		for (alni str_idx = 0; str_idx < len; str_idx++) {
			stroke str = stroke();

			alni p_len; file.read<alni>(&p_len);
			rgba color; file.read<rgba>(&color);

			str.points.Reserve(p_len);
			for (auto piter : str.points) {
				file.read<stroke_point>(&piter.data());
			}

			str.mesh.color = color;
			base->add_stroke(str);
		}
	}
};

namespace StrokesVersion1 {

	alni save_size(strokes_project* proj) {
		alni out = 0;
		out += sizeof(camera);
		out += sizeof(halnf);
		out += sizeof(halnf);
		out += sizeof(rgba);

		out += sizeof(alni);
		for (auto layer : proj->layers) {
			out += layer->name.save_size();
			out += sizeof(bool);
			for (auto stiter : layer->strokes) {
				stroke* str = &stiter.Data();
				out += sizeof(rgba);
				out += sizeof(alni);
				out += str->points.length * sizeof(stroke_point);
			}
		}
		return out;
	}

	void save(File& file, strokes_project* proj) {

		file.write<camera>(&proj->cam);
		file.write<halnf>(&proj->sampler.eraser_size);
		file.write<halnf>(&proj->sampler.screen_thikness);
		file.write<rgba>(&proj->canvas_color);

		alni lay_len = proj->layers.Len();
		file.write<alni>(&lay_len);
		for (auto layer : proj->layers) {
			layer->name.save(&file);

			file.write<bool>(&layer->hiden);

			alni len = layer->strokes.Len();
			file.write<alni>(&len);
			for (auto stiter : layer->strokes) {
				stroke* str = &stiter.Data();

				file.write<rgba>(&str->mesh.color);

				file.write<alni>(&str->points.length);
				for (auto piter : str->points) {
					file.write<stroke_point>(&piter.data());
				}
			}
		}
	}

	void load(File& file, strokes_project* proj) {
		file.read<camera>(&proj->cam);

		file.read<halnf>(&proj->sampler.eraser_size);
		file.read<halnf>(&proj->sampler.screen_thikness);
		file.read<rgba>(&proj->canvas_color);

		alni layers_len;
		file.read<alni>(&layers_len);
		for (alni str_idx = 0; str_idx < layers_len; str_idx++) {

			string key; key.load(&file);
			drawlayer* layer = new drawlayer();
			layer->name = key;
			proj->layers.PushBack(layer);

			file.read<bool>(&layer->hiden);

			alni len;
			file.read<alni>(&len);

			for (alni str_idx = 0; str_idx < len; str_idx++) {
				stroke str = stroke();

				rgba color; file.read<rgba>(&color);

				alni p_len; file.read<alni>(&p_len);
				str.points.Reserve(p_len);
				for (auto piter : str.points) {
					file.read<stroke_point>(&piter.data());
				}

				str.mesh.color = color;
				layer->add_stroke(str);
			}
		}
	}
};
