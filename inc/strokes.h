
#pragma once

#include "glcommon.h"
#include "shader.h"

#include "array.h"
#include "list.h"
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

};

class drawlayer {
	public:

	list<stroke> strokes;
	list<stroke> strokes_undo;

	rgba canvas_color = rgba(0.22f, 0.22f, 0.23f, 1.f);
	void undo();
	void redo();

	void add_stroke(const stroke& str);
	void draw(const mat4f& cammat);
};

class inputsmpler {
	public:

	bool is_active = false;

	stroke input;
	halnf pressure;

	rgba stroke_col = rgba(0.77, 0.77, 0.77, 1);

	halnf screen_precision = 0.009f;
	halnf precision = 0.004;
	halnf screen_thikness = 0.02f;
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
	const stroke& get_stroke();
};

struct strokes_project {

	camera cam;
	drawlayer layer;
	inputsmpler sampler;

	strokes_project() {
		cam.lookat({0, 0, 0}, {100, 0, 0}, {0, 0, 1});
	}

	struct ProjectInfo {
		char name[10] = {0};
		char version[10] = {0};

		ProjectInfo(bool default_val = true) {
			if (default_val) {
				memcp(&name, "strokes", slen("strokes") + 1);
				memcp(&version, "0", slen("0") + 1);
			}
		}
	};

	alni save_size() {
		alni out = 0;
		out += sizeof(ProjectInfo);
		out += sizeof(camera);
		out += sizeof(rgba);

		out += sizeof(alni);

		for (auto stiter : layer.strokes) {
			stroke* str = &stiter.Data();

			out += sizeof(alni);
			out += sizeof(rgba);

			out += str->points.length * sizeof(stroke_point);
		}
		return out;
	}

	void save(File& file) {
		ProjectInfo head;
		file.write<ProjectInfo>(&head);

		file.write<camera>(&cam);

		file.write<rgba>(&layer.canvas_color);

		alni len = layer.strokes.Len();
		file.write<alni>(&len);
		for (auto stiter : layer.strokes) {
			stroke* str = &stiter.Data();

			file.write<alni>(&str->points.length);
			file.write<rgba>(&str->mesh.color);

			for (auto piter : str->points) {
				file.write<stroke_point>(&piter.data());
			}
		}
	}

	void load(File& file) {
		ProjectInfo head(false);
		file.read<ProjectInfo>(&head);
		if (!memequal(head.name, "strokes", slen("strokes"))) {
			return;
		}

		file.read<camera>(&cam);

		file.read<rgba>(&layer.canvas_color);

		alni len = layer.strokes.Len();
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
			layer.add_stroke(str);
		}
	}

	~strokes_project() {
	}
};