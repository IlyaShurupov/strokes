#version 330 core

layout(location = 0) in vec3 str_vert;

out vec4 fragmentColor;

uniform mat4 MVP; 
uniform vec4 StrokeColor; 

void main() {
	gl_Position = MVP * vec4(str_vert, 1);
	fragmentColor = StrokeColor;
}