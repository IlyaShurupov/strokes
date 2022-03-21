
#pragma once

#include "glcommon.h"
#include "shader.h"

#include "array.h"
#include "list.h"

struct camera {

	vec3 pos;
	vec3 target;

	bool orto;

	float fov;
	float near;
	float far;
	float ratio;

	camera();
	void reset();
	mat4 projmat();
	mat4 viewmat();
};

struct stroke_mesh {

	Array<vec3> vbo; // position

	mat4 omatrix = glm::mat4(1.f);

	ogl::shader* shader;
	GLuint VertexArrayID;
	GLuint vertexbuffer;
	GLuint MatrixID;
	GLuint ColorID;

	vec4 color = vec4(1);

	void init();

	stroke_mesh();
	void operator=(const stroke_mesh& in);

	void bind_buffers();
	void draw_mesh(const mat4& proj_mat, const mat4& view_mat);
	~stroke_mesh();
};

struct stroke_point {

	vec3 pos;
	vec4 col;
	vec3 normal;
	float thikness;

	stroke_point();
};

struct stroke {

	Array<stroke_point> points;

	stroke_mesh mesh;

	void gen_quad(alni pidx, stroke_point* p1, stroke_point* p2, vec3 dir1, vec3 dir2);
	vec3 split_dir(vec3 v1, vec3 v2, const vec3& norm);
	
	void gen_mesh();

	void drawcall(const mat4& proj_mat, const mat4& view_mat);
	void add_point(const stroke_point& p);

};

class drawlayer {

	list<stroke> strokes;

public:

	void add_stroke(const stroke& str);
	void draw(const mat4& proj_mat, const mat4& view_mat);
};

class inputsmpler {

	enum class pstate {
		NONE,
		ACTIVE,
	} state = pstate::NONE;

	stroke input;
	float precision = 0.02;
	float pressure;

	void add_point(const vec3& pos, const vec3& norm, float thickness = 0.03);
	vec3 project_3d(const vec2& cpos, camera* cam);
	bool passed(const vec3& point);
	void start(const vec2& cpos, camera* cam);
	void sample_util(const vec2& cpos, camera* cam);
	void finish(const vec2& cpos, camera* cam);

public:

	// cpos - normilized coordinates from center
	void sample(vec2 curs, float pressure, camera* cam);
	void draw(const mat4& proj_mat, const mat4& view_mat);

	bool active_state();
	bool has_input();
	void clear();
	const stroke& get_stroke();
};