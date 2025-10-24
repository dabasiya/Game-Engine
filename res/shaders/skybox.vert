#version 330 core

layout (location = 0) in vec3 apos;

uniform mat4 projection, view;


out vec3 texcoords;

void main() {
	vec4 pos = projection * view * vec4(apos, 0.0);
	gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
	texcoords = vec3(apos.x, apos.y, apos.z);
}