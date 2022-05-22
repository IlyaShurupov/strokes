
#pragma once

#include "glcommon.h"
#include "shader.h"

#include "array.h"
#include "list.h"
#include "map.h"
#include "topology.h"

struct stroke_mesh {

	tp::Array<tp::vec3f> vbo; // position

	tp::ogl::shader* shader;
	GLuint VertexArrayID;
	GLuint vertexbuffer;
	GLuint MatrixID;
	GLuint ColorID;

	tp::rgba color = tp::rgba(1);

	void init();

	stroke_mesh();
	void operator=(const stroke_mesh& in);

	void bind_buffers();
	void draw_mesh(const tp::mat4f& cammat);
	~stroke_mesh();
};

struct stroke_point {

	tp::vec3f pos;
	tp::vec3f normal;
	float thikness;

	stroke_point();
};

struct stroke {

	tp::Array<stroke_point> points;

	stroke_mesh mesh;

	void gen_quad(tp::alni pidx, stroke_point* p1, stroke_point* p2, tp::vec3f dir1, tp::vec3f dir2);
	tp::vec3f split_dir(tp::vec3f v1, tp::vec3f v2, const tp::vec3f& norm);

	void gen_mesh();

	void drawcall(const tp::mat4f& cammat);
	void add_point(const stroke_point& p);

	void denoise_positions(tp::alni passes) {
		for (auto pass : tp::Range(passes)) {
			for (auto pi : tp::Range(points.length() - 2)) {
				points[pi + 1].pos = (points[pi].pos + points[pi + 2].pos) / 2.f;
			}
		}
	}

	void denoise_thickness(tp::alni passes) {
		for (auto pass : tp::Range(passes)) {
			for (auto pi : tp::Range(points.length() - 2)) {
				points[pi + 1].thikness = (points[pi].thikness + points[pi + 2].thikness) / 2.f;
			}
		}
	}

	void reduce_nof_points(tp::halnf pass_factor) {
		if (points.length() < 3) {
			return;
		}

		tp::List<stroke_point> passed_poits;

		for (auto idx : tp::Range(points.length())) {
			passed_poits.pushBack({points[idx]});
		}

		tp::list_node<stroke_point>* min_node = NULL;
		do {
			min_node = NULL;
			tp::halnf min_factor = pass_factor;

			tp::list_node<stroke_point>* iter = passed_poits.first()->next;
			for (; iter->next; iter = iter->next) {
				tp::vec3f dir1 = (iter->data.pos - iter->prev->data.pos).normalize();
				tp::vec3f dir2 = (iter->next->data.pos - iter->data.pos).normalize();
				tp::halnf factor = 1 - dir1.dot(dir2);

				if (factor < min_factor) {
					min_node = iter;
					min_factor = factor;
				}
			}

			if (min_node) {
				passed_poits.delNode(min_node);
			}
		} while (min_node);

		
		points.reserve(passed_poits.length());
		for (auto point : passed_poits) {
			points[point.Idx()] = point.Data();
		}
	}
};

class drawlayer {
	public:
	tp::string name = "new layer";
	tp::List<stroke> strokes;
	tp::List<stroke> strokes_undo;
	bool hiden = false;

	void undo();
	void redo();

	void add_stroke(const stroke& str);
	void draw(const tp::mat4f& cammat);

	void clear_history() {
		strokes_undo.free();
	}

	void clear_canvas() {
		tp::alni len = strokes.length();
		for (tp::alni idx = 0; idx < len; idx++) {
			undo();
		}
	}

	void reduce_size(tp::halnf pass_factor) {
		for (auto str : strokes) {
			str.Data().reduce_nof_points(pass_factor);
			str.Data().gen_mesh();
		}
	}

	void clear() {
		strokes.free();
		strokes_undo.free();
	}
};

class inputsmpler {
	public:

	bool is_active = false;

	stroke input;
	tp::halnf pressure;

	tp::rgba stroke_col = tp::rgba(0.77, 0.77, 0.77, 1);

	tp::halnf screen_precision = 0.000f;
	tp::halnf precision = 0.002;
	tp::halnf screen_thikness = 0.01f;
	tp::halnf thickness = 0.04;

	bool eraser = false;
	tp::halnf eraser_size = 0.1f;

	void add_point(const tp::vec3f& pos, const tp::vec3f& norm, float thickness);
	bool passed(const tp::vec3f& point);
	void start(const tp::vec2f& cpos, tp::Camera* cam);
	void sample_util(const tp::vec2f& cpos, tp::Camera* cam);
	void erase_util(tp::List<stroke>* pull, tp::List<stroke>* undo, const tp::vec2f& cpos, tp::Camera* cam);
	void finish(const tp::vec2f& cpos, tp::Camera* cam);

	void sample(tp::List<stroke>* pull, tp::List<stroke>* undo, tp::vec2f curs, float pressure, tp::Camera* cam);
	void draw(const tp::mat4f& cammat);

	bool active_state();
	bool has_input();
	void clear();
	stroke& get_stroke();
};

struct strokes_project {

	tp::Camera cam;
	drawlayer* active_layer = NULL;
	tp::List<drawlayer*> layers;
	inputsmpler sampler;
	tp::rgba canvas_color = tp::rgba(0.22f, 0.22f, 0.25f, 1.f);

	tp::halni denoise_passes = 1;
	tp::halni denoise_passes_thikness = 3;
	bool auto_reduction = true;
	tp::halnf pass_factor = tp::halnf(0.001);

	strokes_project() {
		cam.lookat({0, 0, 0}, {100, 0, 0}, {0, 0, 1});
	}

	drawlayer* get_base_layer() {
		if (!layers.length()) {
			layers.pushBack(new drawlayer());
		}
		return layers[0];
	}

	void append_layers(drawlayer* base) {
		for (auto lay : layers) {
			base->strokes += lay->strokes;
		}
	}

	tp::alni save_size();
	void save(tp::File& file);
	void load(tp::File& file);

	~strokes_project() {
		for (auto lay : layers) {
			delete lay.Data();
		}
	}
};