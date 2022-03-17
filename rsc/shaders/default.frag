#version 330 core

in vec3 fragmentColor;
in vec3 v_barycentric;

layout(location = 0) out vec4 color;

void main(){

	float f_thickness = 0.01;

	float f_closest_edge = min(v_barycentric.x, min(v_barycentric.y, v_barycentric.z)); // see to which edge this pixel is the closest
	float f_width = fwidth(f_closest_edge); // calculate derivative (divide f_thickness by this to have the line width constant in screen-space)
	float f_alpha = smoothstep(f_thickness, f_thickness + f_width, f_closest_edge); // calculate alpha
	
	color = vec4(vec3(f_alpha), 1);

}