#version 330 core

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

layout (location = 0) out vec3 PositionBuffer;
layout (location = 1) out vec3 NormalBuffer;
layout (location = 2) out vec4 Albedo;

in vec3 fragpos;

in vec3 Normal;

in vec2 texcoords;

void main() {
	PositionBuffer = fragpos;
	NormalBuffer = (Normal / 2) + 0.5;

	Albedo = texture(texture_diffuse1, texcoords);
}