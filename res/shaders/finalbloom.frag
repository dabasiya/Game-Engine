#version 330 core

uniform sampler2D i1;
uniform sampler2D i2;

in vec2 texcoords;

out vec4 Fragcolor;

void main() {
	vec4 t1 = texture(i1, texcoords);
	vec4 t2 = texture(i2, texcoords);
	Fragcolor = (t1+t2)/2.0;
}