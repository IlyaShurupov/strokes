
#include "strokes.h"

camera::camera() {
	reset();
}

void camera::reset() {
	pos = vec3(0, 0, 4);
	target = vec3(0, 0, 0);

	fov = 45.f;
	near = 0.01f;
	far = 100.f;
	orto = false;
	ratio = 4 / 3.f;
}

mat4 camera::projmat() {
	return  glm::perspective(glm::radians((float)fov), (float)ratio, (float)near, (float)far);
}

mat4 camera::viewmat() {
	return glm::lookAt(pos, target, glm::vec3(0, 1, 0));
}


void stroke_mesh::init() {
	static ogl::shader shader("stroke", NULL, "stroke");
	this->shader = &shader;

	MatrixID = shader.getu("MVP");
	ColorID = shader.getu("StrokeColor");

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexbuffer);
}

stroke_mesh::stroke_mesh() {
	init();
}

void stroke_mesh::operator=(const stroke_mesh& in) {
	
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);

	init();

	vbo = in.vbo;
	omatrix = in.omatrix;

	bind_buffers();
}

void stroke_mesh::bind_buffers() {

	glBindVertexArray(VertexArrayID);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vbo[0]) * vbo.length, vbo.buff, GL_STATIC_DRAW);
}


void stroke_mesh::draw_mesh(const mat4& proj_mat, const mat4& view_mat) {
	
	glBindVertexArray(VertexArrayID);

	shader->bind();
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(proj_mat * view_mat)[0][0]);
	glUniform4fv(ColorID, 1, &color.x);

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Draw the triangles ! // mode. count. type. element. array buffer offset
	glDrawArrays(GL_TRIANGLES, 0, vbo.length);

	glDisableVertexAttribArray(0);

	shader->unbind();
}

stroke_mesh::~stroke_mesh() {
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
}


stroke_point::stroke_point() {
	pos = vec3(0, 0, 0);
	col = vec4(1, 1, 1, 1);
	normal = vec3(0, 1, 0);
	thikness = 0.06;
}

void stroke::gen_quad(alni pidx, stroke_point* p1, stroke_point* p2, vec3 dir1, vec3 dir2) {
	/*
		vec3 perp = normalize(cross(p1->normal, p2->pos - p1->pos)) * p1->thikness;
		float cosa1 = dot(perp, dir1) / (dir1.length() * perp.length());
		float cosa2 = dot(perp, dir2) / (dir2.length() * perp.length());
		float thickness1 = -p1->thikness / cosa1;
		float thickness2 = -p2->thikness / cosa2;
	*/

	vec3 v1 = p1->pos + dir1 * p1->thikness;
	vec3 v2 = p2->pos + dir2 * p2->thikness;
	vec3 v3 = p2->pos - dir2 * p2->thikness;
	vec3 v4 = p1->pos - dir1 * p1->thikness;

	alni vidx = pidx * 6;

	mesh.vbo[vidx] = v1;
	mesh.vbo[vidx + 1] = v2;
	mesh.vbo[vidx + 2] = v3;
	mesh.vbo[vidx + 3] = v3;
	mesh.vbo[vidx + 4] = v4;
	mesh.vbo[vidx + 5] = v1;
}

vec3 stroke::split_dir(vec3 v1, vec3 v2, const vec3& norm) {

	v1 = normalize(v1);
	v2 = normalize(v2);

	vec3 plane_normal;

	float val = glm::dot(v2, v1);
	if (val > -0.999999) {
		vec3 crossp = cross(v1, v2);
		float rot_angle = acos(glm::dot(v1, v2)) / 2.f;
		mat4 rot_matrix = rotate(rot_angle, crossp);
		vec3 middle = rot_matrix * vec4(v1.x, v1.y, v1.z, 0);
		plane_normal = cross(middle, crossp);
	}
	else {
		plane_normal = v2;
	}

	return glm::cross(plane_normal, norm);
}

void stroke::gen_mesh() {

	alni nvert = (points.length - 1) * 6;

	mesh.vbo.Reserve(nvert);

	for (alni pidx = 0; pidx < points.length - 1; pidx++) {

		stroke_point pt0;
		stroke_point pt1 = points[pidx];
		stroke_point pt2 = points[pidx + 1];
		stroke_point pt3;

		glm::vec3 dir1;
		glm::vec3 dir2;

		if (pidx > 0) {
			pt0 = points[pidx - 1];
		}
		else {
			pt0.pos = pt1.pos + (pt1.pos - pt2.pos);
			pt0.thikness = 0.001;
		}

		if (pidx < points.length - 2) {
			pt3 = points[pidx + 2];
		}
		else {
			pt3.pos = pt2.pos + (pt2.pos - pt1.pos);
			pt3.thikness = 0.001;
		}

		dir1 = split_dir(pt0.pos - pt1.pos, pt2.pos - pt1.pos, pt1.normal);
		dir2 = split_dir(pt1.pos - pt2.pos, pt3.pos - pt2.pos, pt2.normal);

		if (glm::dot(dir1, dir2) < 0) {
			dir1 *= -1;
		}

		gen_quad(pidx, &pt1, &pt2, normalize(dir1), normalize(dir2));
	}

	mesh.bind_buffers();
}

void stroke::drawcall(const mat4& proj_mat, const mat4& view_mat) {
	mesh.draw_mesh(proj_mat, view_mat);
}

void stroke::add_point(const stroke_point& p) {
	points.PushBack(p);
}


void drawlayer::add_stroke(const stroke& str) {
	strokes.PushBack(str);
	// strokes.Last()->data.gen_mesh();
}

void drawlayer::draw(const mat4& proj_mat, const mat4& view_mat) {
	for (auto str : strokes) {
		str.Data().drawcall(proj_mat, view_mat);
	}
}

void inputsmpler::add_point(const vec3& pos, const vec3& norm, float thickness) {
	stroke_point p;
	p.pos = pos;
	p.normal = norm;
	p.thikness = thickness * pressure;
	input.add_point(p);
}

vec3 inputsmpler::project_3d(const vec2& cpos, camera* cam) {
	vec3 forw = normalize(cam->target - cam->pos);
	vec3 right = normalize(cross(forw, vec3(0, 1, 0)));
	vec3 up = cross(right, forw);

	float scale = tan(radians(cam->fov) / 2) * length(cam->target - cam->pos);
	vec3 out = cam->target + (cpos.x * scale * right * cam->ratio) + (cpos.y * scale * up);
	return out;
}

bool inputsmpler::passed(const vec3& point) {
	if (input.points.length) {
		return length(point - input.points[input.points.length - 1].pos) > precision;
	}
	return true;
}

void inputsmpler::start(const vec2& cpos, camera* cam) {
	input.points.Free();
	vec3 point = project_3d(cpos, cam);
	add_point(point, normalize(cam->target - cam->pos), 0.01);
}

void inputsmpler::sample_util(const vec2& cpos, camera* cam) {
	vec3 point = project_3d(cpos, cam);
	if (passed(point)) {
		add_point(point, normalize(cam->target - cam->pos));
	}
}

void inputsmpler::finish(const vec2& cpos, camera* cam) {
	if (input.points.length == 1) {
		input.points.Free();
	}
	else {
		input.points[input.points.length - 1].thikness = 0.01;
	}
}

void inputsmpler::sample(vec2 curs, float pressure, camera* cam) {

	this->pressure = pressure;

	switch (state) {
		case pstate::NONE: {
			if (pressure > 0) {
				start(curs, cam);
				state = pstate::ACTIVE;
			}
			return;
		}
		case pstate::ACTIVE: {
			if (!pressure > 0) {
				finish(curs, cam);
				state = pstate::NONE;
				return;
			}
			sample_util(curs, cam);
			return;
		}
	}
}

void inputsmpler::draw(const mat4& proj_mat, const mat4& view_mat) {
	static int prev_len = input.points.length;

	if (input.points.length > 1) {
		if (prev_len < input.points.length) {
			input.gen_mesh();
		}
		input.drawcall(proj_mat, view_mat);
	}
	prev_len = input.points.length;
}

bool inputsmpler::active_state() {
	return state == pstate::ACTIVE;
}
bool inputsmpler::has_input() {
	return input.points.length > 1;
}
void inputsmpler::clear() {
	input.points.Free();
	state = pstate::NONE;
}
const stroke& inputsmpler::get_stroke() {
	return input;
}