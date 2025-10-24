#version 330 core


in vec3 fragPos;

uniform vec3 lightpos;
uniform float farplane;

void main() {
	gl_FragDepth = length(fragPos - lightpos) / farplane;
}