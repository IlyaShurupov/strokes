
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

	void undo();
	void redo();

	void add_stroke(const stroke& str);
	void draw(const mat4f& cammat);
};

class inputsmpler {
public:

	enum class pstate {
		NONE,
		ACTIVE,
	} state = pstate::NONE;

	stroke input;
	float pressure;

	halnf precision = 0.02;
	float thickness = 0.04;
	rgba stroke_col = rgba(0.77, 0.77, 0.77, 1);

	bool eraser = false;
	float eraser_size = 0.1f;

	void add_point(const vec3f& pos, const vec3f& norm, float thickness);
	bool passed(const vec3f& point);
	void start(const vec2f& cpos, camera* cam);
	void sample_util(const vec2f& cpos, camera* cam);
	void erase_util(list<stroke>* pull, list<stroke>* undo, const vec2f& cpos, camera* cam);
	void finish(const vec2f& cpos, camera* cam);

	// cpos - normilized coordinates from center
	void sample(list<stroke>* pull, list<stroke>* undo, vec2f curs, float pressure, camera* cam);
	// screen space
	void draw(const mat4f& cammat);

	bool active_state();
	bool has_input();
	void clear();
	const stroke& get_stroke();
};