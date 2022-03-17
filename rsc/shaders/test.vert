#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 barymetric_coords;

out vec3 fragmentColor;
out vec3 v_barycentric;

uniform mat4 MVP; 

void main() {
	gl_Position = MVP * vec4(vertexPosition_modelspace, 1);
	fragmentColor = vertexColor;
	v_barycentric = barymetric_coords;
}