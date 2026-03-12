#version 330 core

in vec2 texcoords;

out vec4 outputimage;

uniform sampler2D imagetex;

void main() {
	outputimage = texture(imagetex, texcoords);
}