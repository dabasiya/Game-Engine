#version 330 core
layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anormal;
layout (location = 2) in vec2 atexcoords;
layout (location = 5) in vec4 acolor;
layout (location = 6) in vec4 boneid;
layout (location = 7) in vec4 weight;

out vec2 texcoords;

out vec4 colors;

out vec3 Normal;

uniform mat4 model, viewproj;

out vec3 fragpos;



void main() {
	texcoords = atexcoords;
	colors = acolor;

	fragpos = vec3(model * vec4(apos, 1.0));

	Normal = mat3(transpose(inverse(model))) * anormal;

	gl_Position = viewproj * model * vec4(apos, 1.0);
}