#version 330 core

out vec4 Fragcolor;

in vec3 texcoords;

uniform samplerCube scube;

void main() {
	Fragcolor = texture(scube, texcoords);
}