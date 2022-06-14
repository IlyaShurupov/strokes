#pragma once 

#include "strokes.h"

struct ProjectInfo {
	char name[10] = {0};
	char version[10] = {0};
	
	void init(tp::alni v) {
		tp::string version_s(v);
		tp::memcp(name, "strokes", tp::slen("strokes"));
		tp::memcp(version, version_s.cstr(), version_s.size());
	}
};

namespace StrokesVersion0 {

	tp::alni save_size(strokes_project* proj) {
		tp::alni out = 0;
		out += sizeof(tp::Camera);
		out += sizeof(tp::rgba);

		out += sizeof(tp::alni);
		for (auto layer : proj->layers) {
			for (auto stiter : layer->strokes) {
				stroke* str = &stiter.data();
				
				out += sizeof(tp::rgba);

				out += sizeof(tp::alni);
				out += str->points.length() * sizeof(stroke_point);
			}
		}
		return out;
	}

	void save(tp::File& file, strokes_project* proj) {

		file.write<tp::Camera>(&proj->cam);
		file.write<tp::rgba>(&proj->canvas_color);

		drawlayer base_layer;
		proj->append_layers(&base_layer);

		tp::alni len = base_layer.strokes.length();
		file.write<tp::alni>(&len);
		for (auto stiter : base_layer.strokes) {
			stroke* str = &stiter.data();

			tp::alni length = str->points.length();
			file.write<tp::alni>(&length);
			file.write<tp::rgba>(&str->mesh.color);

			for (auto piter : str->points) {
				file.write<stroke_point>(&piter.data());
			}
		}
	}

	void load(tp::File& file, strokes_project* proj) {
		file.read<tp::Camera>(&proj->cam);

		file.read<tp::rgba>(&proj->canvas_color);

		drawlayer* base = proj->get_base_layer();

		tp::alni len;
		file.read<tp::alni>(&len);

		for (tp::alni str_idx = 0; str_idx < len; str_idx++) {
			stroke str = stroke();

			tp::alni p_len; file.read<tp::alni>(&p_len);
			tp::rgba color; file.read<tp::rgba>(&color);

			str.points.reserve(p_len);
			for (auto piter : str.points) {
				file.read<stroke_point>(&piter.data());
			}

			str.mesh.color = color;
			base->add_stroke(str);
		}
	}
};

namespace StrokesVersion1 {

	tp::alni save_size(strokes_project* proj) {
		tp::alni out = 0;
		out += sizeof(tp::Camera);
		out += sizeof(tp::halnf);
		out += sizeof(tp::halnf);
		out += sizeof(tp::rgba);

		out += sizeof(tp::alni);
		for (auto layer : proj->layers) {
			out += layer->name.save_size();
			out += sizeof(bool);
			out += sizeof(tp::alni);
			for (auto stiter : layer->strokes) {
				stroke* str = &stiter.data();
				out += sizeof(tp::rgba);
				out += sizeof(tp::alni);
				out += str->points.length() * sizeof(stroke_point);
			}
		}
		return out;
	}

	void save(tp::File& file, strokes_project* proj) {

		file.write<tp::Camera>(&proj->cam);
		file.write<tp::halnf>(&proj->sampler.eraser_size);
		file.write<tp::halnf>(&proj->sampler.screen_thikness);
		file.write<tp::rgba>(&proj->canvas_color);

		tp::alni lay_len = proj->layers.length();
		file.write<tp::alni>(&lay_len);
		for (auto layer : proj->layers) {
			layer->name.save(&file);

			file.write<bool>(&layer->hiden);

			tp::alni len = layer->strokes.length();
			file.write<tp::alni>(&len);
			for (auto stiter : layer->strokes) {
				stroke* str = &stiter.data();

				file.write<tp::rgba>(&str->mesh.color);

				tp::alni length = str->points.length();
				file.write<tp::alni>(&length);
				for (auto piter : str->points) {
					file.write<stroke_point>(&piter.data());
				}
			}
		}
	}

	void load(tp::File& file, strokes_project* proj) {
		file.read<tp::Camera>(&proj->cam);

		file.read<tp::halnf>(&proj->sampler.eraser_size);
		file.read<tp::halnf>(&proj->sampler.screen_thikness);
		file.read<tp::rgba>(&proj->canvas_color);

		tp::alni layers_len;
		file.read<tp::alni>(&layers_len);
		for (tp::alni str_idx = 0; str_idx < layers_len; str_idx++) {

			tp::string key; key.load(&file);
			drawlayer* layer = new drawlayer();
			layer->name = key;
			proj->layers.pushBack(layer);

			file.read<bool>(&layer->hiden);

			tp::alni len;
			file.read<tp::alni>(&len);

			for (tp::alni str_idx = 0; str_idx < len; str_idx++) {
				stroke str = stroke();

				tp::rgba color; file.read<tp::rgba>(&color);

				tp::alni p_len; file.read<tp::alni>(&p_len);
				str.points.reserve(p_len);
				for (auto piter : str.points) {
					file.read<stroke_point>(&piter.data());
				}

				str.mesh.color = color;
				layer->add_stroke(str);
			}
		}
	}
};
