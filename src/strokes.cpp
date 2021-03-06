
#include "strokes.h"

#include "glutils.h"

#include "strokes_version.h"

using namespace tp;

void stroke_mesh::init() {
	static ogl::shader shader("rsc/shaders/stroke", NULL, "rsc/shaders/stroke");
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
	color = in.color;

	bind_buffers();
}

void stroke_mesh::bind_buffers() {

	glBindVertexArray(VertexArrayID);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vbo[0]) * vbo.length(), vbo.buff(), GL_STATIC_DRAW);
}


void stroke_mesh::draw_mesh(const mat4f& cammat) {

	glBindVertexArray(VertexArrayID);

	shader->bind();
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &cammat[0][0]);
	glUniform4fv(ColorID, 1, &color.rgbs.r);

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	// Draw the triangles ! // mode. count. type. element. array buffer offset
	glDrawArrays(GL_TRIANGLES, 0, vbo.length());

	glDisableVertexAttribArray(0);

	shader->unbind();
}

stroke_mesh::~stroke_mesh() {
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
}


stroke_point::stroke_point() {
	pos = vec3f(0, 0, 0);
	normal = vec3f(0, 1, 0);
	thikness = 0.06;
}

void stroke::gen_quad(alni pidx, stroke_point* p1, stroke_point* p2, vec3f dir1, vec3f dir2) {
	/*
		vec3 perp = normalize(cross(p1->normal, p2->pos - p1->pos)) * p1->thikness;
		float cosa1 = dot(perp, dir1) / (dir1.length() * perp.length());
		float cosa2 = dot(perp, dir2) / (dir2.length() * perp.length());
		float thickness1 = -p1->thikness / cosa1;
		float thickness2 = -p2->thikness / cosa2;
	*/

	vec3f v1 = p1->pos + dir1 * p1->thikness;
	vec3f v2 = p2->pos + dir2 * p2->thikness;
	vec3f v3 = p2->pos - dir2 * p2->thikness;
	vec3f v4 = p1->pos - dir1 * p1->thikness;

	alni vidx = pidx * 6;

	mesh.vbo[vidx] = v1;
	mesh.vbo[vidx + 1] = v2;
	mesh.vbo[vidx + 2] = v3;
	mesh.vbo[vidx + 3] = v3;
	mesh.vbo[vidx + 4] = v4;
	mesh.vbo[vidx + 5] = v1;
}

vec3f stroke::split_dir(vec3f v1, vec3f v2, const vec3f& norm) {

	v1.normalize();
	v2.normalize();

	vec3f plane_normal;

	halnf val = v2.dot(v1);

	if (val > -0.999999) {
		vec3f crossp = v1.cross(v2);
		mat3f rot_matrix = mat3f::rotmat(crossp, acos(v1.dot(v2)) / 2.f);
		vec3f middle = rot_matrix * v1;
		plane_normal = middle.cross(crossp);
	} else {
		plane_normal = v2;
	}

	return plane_normal.cross(norm);
}

void stroke::gen_mesh() {

	alni nvert = (points.length() - 1) * 6;

	mesh.vbo.reserve(nvert);

	for (alni pidx = 0; pidx < points.length() - 1; pidx++) {

		stroke_point pt0;
		stroke_point pt1 = points[pidx];
		stroke_point pt2 = points[pidx + 1];
		stroke_point pt3;

		vec3f dir1;
		vec3f dir2;

		if (pidx > 0) {
			pt0 = points[pidx - 1];
		} else {
			pt0.pos = pt1.pos + (pt1.pos - pt2.pos);
			pt0.thikness = 0.001;
		}

		if (pidx < points.length() - 2) {
			pt3 = points[pidx + 2];
		} else {
			pt3.pos = pt2.pos + (pt2.pos - pt1.pos);
			pt3.thikness = 0.001;
		}

		dir1 = split_dir(pt0.pos - pt1.pos, pt2.pos - pt1.pos, pt1.normal);
		dir2 = split_dir(pt1.pos - pt2.pos, pt3.pos - pt2.pos, pt2.normal);

		if (dir1.dot(dir2) < 0) {
			dir1 *= -1;
		}

		gen_quad(pidx, &pt1, &pt2, dir1.unitv(), dir2.unitv());
	}

	mesh.bind_buffers();
}

void stroke::drawcall(const mat4f& cammat) {
	mesh.draw_mesh(cammat);
}

void stroke::add_point(const stroke_point& p) {
	points.pushBack(p);
}


void drawlayer::undo() {
	if (strokes.last()) {
		strokes_undo.pushBack(strokes.last()->data);
		strokes.delNode(strokes.last());
	}
}

void drawlayer::redo() {
	if (strokes_undo.last()) {
		strokes.pushBack(strokes_undo.last()->data);
		strokes_undo.delNode(strokes_undo.last());
	}
}

void drawlayer::add_stroke(const stroke& str) {
	strokes.pushBack(str);
	strokes.last()->data.gen_mesh();
}

void drawlayer::draw(const mat4f& cammat) {
	for (ListNode<stroke>* str = strokes.last(); str; str = str->prev) {
		str->data.drawcall(cammat);
	}
}

void inputsmpler::add_point(const vec3f& pos, const vec3f& norm, float thickness) {
	stroke_point p;
	p.pos = pos;
	p.normal = norm;
	p.thikness = thickness * pressure;
	input.add_point(p);
}

bool inputsmpler::passed(const vec3f& point) {
	if (input.points.length()) {
		return (point - input.points[input.points.length() - 1].pos).length() > precision;
	}
	return true;
}

void inputsmpler::start(const vec2f& cpos, Camera* cam) {
	input.points.free();
	vec3f point = cam->project(cpos);
	add_point(point, cam->get_fw(), 0.001f);
}

void inputsmpler::sample_util(const vec2f& cpos, Camera* cam) {
	vec3f point = cam->project(cpos);
	if (passed(point)) {
		add_point(point, cam->get_fw(), thickness);
	}
}

void inputsmpler::erase_util(List<stroke>* pull, List<stroke>* undo, const vec2f& cpos, Camera* cam) {

	ListNode<stroke>* str = pull->first();
	while (str) {
		bool remove = false;

		for (auto pnt : str->data.points) {
			if ((cam->project(pnt.data().pos) - cpos).length() < eraser_size) {
				remove = true;
				break;
			}
		}

		if (remove) {
			ListNode<stroke>* tmp = str->next;
			undo->pushBack(str->data);
			pull->delNode(str);
			str = tmp;
		} else {
			str = str->next;
		}
	}
}

void inputsmpler::finish(const vec2f& cpos, Camera* cam) {
	if (input.points.length() <= 1) {
		input.points.free();
	} else {
		input.points[input.points.length() - 1].thikness = 0.001f;
		cam->offset_target(0.0001);
	}
}

void inputsmpler::sample(List<stroke>* pull, List<stroke>* undo, vec2f curs, float pressure, Camera* cam) {

	this->input.mesh.color = stroke_col;
	this->pressure = pressure;

	if (eraser) {
		if (pressure > 0) {
			input.points.free();
			erase_util(pull, undo, curs, cam);
		}
		return;
	} else {
		vec3f zero_projection = cam->project(vec2f(0.f));
		thickness = (cam->project(vec2f(0.f, screen_thikness)) - zero_projection).length();
		precision = (cam->project(vec2f(0.f, screen_precision)) - zero_projection).length();
	}

	switch (is_active) {
		case false:
		{
			if (pressure) {
				start(curs, cam);
				is_active = true;
			}
			return;
		}
		case true:
		{
			sample_util(curs, cam);
			if (!pressure) {
				finish(curs, cam);
				is_active = false;
				return;
			}
			return;
		}
	}
}

void inputsmpler::draw(const mat4f& cammat) {

	static int prev_len = input.points.length();
	if (input.points.length() > 1) {
		if (prev_len < input.points.length()) {
			input.gen_mesh();
		}
		input.drawcall(cammat);
	}
	prev_len = input.points.length();
}

bool inputsmpler::active_state() {
	return is_active;
}
bool inputsmpler::has_input() {
	return input.points.length() > 1;
}
void inputsmpler::clear() {
	input.points.free();
	is_active = false;
}
stroke& inputsmpler::get_stroke() {
	return input;
}


alni strokes_project::save_size() {
	alni version_size = StrokesVersion1::save_size(this);
	return version_size + sizeof(ProjectInfo);
}

void strokes_project::save(File& file) {
	ProjectInfo head;
	head.init(1);
	file.write<ProjectInfo>(&head);
	StrokesVersion1::save(file, this);
}

void strokes_project::load(File& file) {
	ProjectInfo head;
	file.read<ProjectInfo>(&head);

	if (string(head.name) != "strokes") {
		return;
	}

	alni version = alni(string(head.version));
	if (version > 1 || version < 0) {
		return;
	}

	version == 1 ? StrokesVersion1::load(file, this) : StrokesVersion0::load(file, this);

	active_layer = get_base_layer();
}