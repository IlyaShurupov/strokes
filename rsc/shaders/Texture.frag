#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D renderedTexture;
uniform float time;

void main(){
	vec4 texColor = texture(renderedTexture, UV);
	if(texColor.a < 0.2)
        discard;

	color = texColor;
}
