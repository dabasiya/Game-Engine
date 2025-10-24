#version 330 core


layout (location = 0) in vec3 apos;

uniform mat4 viewproj, model;

void main() {
	gl_Position = viewproj * model * vec4(apos, 1.0f);
}