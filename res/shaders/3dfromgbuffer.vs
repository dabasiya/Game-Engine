#version 330 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec2 atexcoords;
layout (location = 5) in vec4 acolor;

out vec2 texcoords;

out vec4 colors;

void main() {
	gl_Position = vec4(apos, 1.0);
	texcoords = atexcoords;
	colors = acolor;
}