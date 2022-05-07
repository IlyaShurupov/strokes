
#pragma once

#include "glcommon.h"
#include "shader.h"

#include "array.h"
#include "list.h"
#include "map.h"
#include "topology.h"

struct stroke_mesh {

	Array<vec3f> vbo; // position

	ogl::shader* shader;
	GLuint VertexArrayID;
	GLuint vertexbuffer;
	GLuint MatrixID;
	GLuint ColorID;

	rgba color = rgba(1);

	void init();

	stroke_mesh();
	void operator=(const stroke_mesh& in);

	void bind_buffers();
	void draw_mesh(const mat4f& cammat);
	~stroke_mesh();
};

struct stroke_point {

	vec3f pos;
	vec3f normal;
	float thikness;

	stroke_point();
};

struct stroke {

	Array<stroke_point> points;

	stroke_mesh mesh;

	void gen_quad(alni pidx, stroke_point* p1, stroke_point* p2, vec3f dir1, vec3f dir2);
	vec3f split_dir(vec3f v1, vec3f v2, const vec3f& norm);

	void gen_mesh();

	void drawcall(const mat4f& cammat);
	void add_point(const stroke_point& p);

	void denoise_positions(alni passes) {
		for (auto pass : range(passes)) {
			for (auto pi : range(points.Len() - 2)) {
				points[pi + 1].pos = (points[pi].pos + points[pi + 2].pos) / 2.f;
			}
		}
	}

	void denoise_thickness(alni passes) {
		for (auto pass : range(passes)) {
			for (auto pi : range(points.Len() - 2)) {
				points[pi + 1].thikness = (points[pi].thikness + points[pi + 2].thikness) / 2.f;
			}
		}
	}

	void reduce_nof_points(halnf pass_factor) {
		if (points.length < 3) {
			return;
		}

		list<stroke_point> passed_poits;

		for (auto idx : range(points.length)) {
			passed_poits.PushBack({points[idx]});
		}

		list_node<stroke_point>* min_node = NULL;
		do {
			min_node = NULL;
			halnf min_factor = pass_factor;

			list_node<stroke_point>* iter = passed_poits.First()->next;
			for (; iter->next; iter = iter->next) {
				vec3f dir1 = (iter->data.pos - iter->prev->data.pos).normalize();
				vec3f dir2 = (iter->next->data.pos - iter->data.pos).normalize();
				halnf factor = 1 - dir1.dot(dir2);

				if (factor < min_factor) {
					min_node = iter;
					min_factor = factor;
				}
			}

			if (min_node) {
				passed_poits.DelNode(min_node);
			}
		} while (min_node);

		
		points.Reserve(passed_poits.Len());
		for (auto point : passed_poits) {
			points[point.Idx()] = point.Data();
		}
	}
};

class drawlayer {
	public:
	string name = "new layer";
	list<stroke> strokes;
	list<stroke> strokes_undo;
	bool hiden = false;

	void undo();
	void redo();

	void add_stroke(const stroke& str);
	void draw(const mat4f& cammat);

	void clear_history() {
		strokes_undo.Clear();
	}

	void clear_canvas() {
		alni len = strokes.Len();
		for (alni idx = 0; idx < len; idx++) {
			undo();
		}
	}

	void reduce_size(halnf pass_factor) {
		for (auto str : strokes) {
			str.Data().reduce_nof_points(pass_factor);
			str.Data().gen_mesh();
		}
	}

	void clear() {
		strokes.Clear();
		strokes_undo.Clear();
	}
};

class inputsmpler {
	public:

	bool is_active = false;

	stroke input;
	halnf pressure;

	rgba stroke_col = rgba(0.77, 0.77, 0.77, 1);

	halnf screen_precision = 0.000f;
	halnf precision = 0.002;
	halnf screen_thikness = 0.01f;
	halnf thickness = 0.04;

	bool eraser = false;
	halnf eraser_size = 0.1f;

	void add_point(const vec3f& pos, const vec3f& norm, float thickness);
	bool passed(const vec3f& point);
	void start(const vec2f& cpos, camera* cam);
	void sample_util(const vec2f& cpos, camera* cam);
	void erase_util(list<stroke>* pull, list<stroke>* undo, const vec2f& cpos, camera* cam);
	void finish(const vec2f& cpos, camera* cam);

	void sample(list<stroke>* pull, list<stroke>* undo, vec2f curs, float pressure, camera* cam);
	void draw(const mat4f& cammat);

	bool active_state();
	bool has_input();
	void clear();
	stroke& get_stroke();
};

struct strokes_project {

	camera cam;
	drawlayer* active_layer = NULL;
	list<drawlayer*> layers;
	inputsmpler sampler;
	rgba canvas_color = rgba(0.22f, 0.22f, 0.25f, 1.f);

	halni denoise_passes = 1;
	halni denoise_passes_thikness = 3;
	bool auto_reduction = true;
	halnf pass_factor = halnf(0.001);

	strokes_project() {
		cam.lookat({0, 0, 0}, {100, 0, 0}, {0, 0, 1});
	}

	drawlayer* get_base_layer() {
		if (!layers.Len()) {
			layers.PushBack(new drawlayer());
		}
		return layers[0];
	}

	void append_layers(drawlayer* base) {
		for (auto lay : layers) {
			base->strokes += lay->strokes;
		}
	}

	alni save_size();
	void save(File& file);
	void load(File& file);

	~strokes_project() {
		for (auto lay : layers) {
			delete lay.Data();
		}
	}
};