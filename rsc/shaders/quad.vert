#version 330 core

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec4 vert_col;

out vec4 fragmentColor;

void main() {
	gl_Position = vec4(vert_pos, 1);
	fragmentColor = vert_col;
}