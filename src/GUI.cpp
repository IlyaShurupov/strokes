
#include "GUI.h"

#include "glutils.h"

#include "SOIL/soil.h"
#include <lunasvg.h>

using namespace lunasvg;

GLuint load_texture(string name) {
	GLuint tex_2d = 0;

	if (0) {
		auto document = Document::loadFromFile("tiger.svg");
		auto bitmap = document->renderToBitmap();
	}
	else {
		tex_2d = SOIL_load_OGL_texture(name.cstr(), SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

		if (0 == tex_2d) {
			printf("SOIL loading error: '%s'\n", SOIL_last_result());
		}
	}

	return tex_2d;
}

GuiState::GuiState(ogl::window* winp) {
	this->win = winp;
}

GLuint GuiState::get_tex(const char* TexId) {
	GLuint out = 0;
	alni idx = textures.Presents(TexId);
	if (idx != -1) {
		out = textures.Get(TexId);
	}
	else {
		out = load_texture(TexId);
		textures.Put(TexId, out);
	}
	return out;
}

void GuiState::convert_rect(vec4& rec) {
	rec.y = win->size.y - rec.y;
}

bool GuiState::inside(const vec4& rec) {
	vec2 cur = win->cursor();
	cur.y = win->size.y - cur.y;
	return rec.x < cur.x && rec.y < cur.y && rec.x + rec.z > cur.x && rec.y + rec.w > cur.y;
}

bool GuiState::pushed(const vec4& rec) {
	return inside(rec) && win->rmb();
}

void GuiState::Icon(vec4 rect, const char* IconId) {
	GLuint tex = get_tex(IconId);
	if (tex) {
		draw_texture(0, tex, vec4(0, 0, win->size.x, win->size.y), rect);
	}
}

bool GuiState::button(vec4 rect, const char* name, const char* IconId) {

	Icon(rect, IconId);

	if (pushed(rect)) {
		return true;
	}
	return false;
}

GuiState::~GuiState() {
	for (auto tex : textures) {
		glDeleteTextures(1, &tex->val);
	}
}
