
#pragma once

#include "glcommon.h"

void init_utils();
void finalize_utils();

void draw_texture(GLuint out, GLuint in, const vec4& rec_domen, const vec4& rec_target);
GLuint get_tex(const char* TexId);