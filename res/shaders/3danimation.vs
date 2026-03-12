#version 330 core
layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anormal;
layout (location = 2) in vec2 atexcoords;
layout (location = 5) in vec4 acolors;
layout (location = 6) in vec4 boneid;
layout (location = 7) in vec4 weight;

out vec2 texcoords;
out vec3 Normal, fragpos;

out vec4 colors;

uniform mat4 model, viewproj;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalbonesmetrices[MAX_BONES];


void main() {
	texcoords = atexcoords;

	vec4 totalposition = vec4(0.0);
	for(int i = 0; i<MAX_BONE_INFLUENCE; i++) {
		if(int(boneid[i]) == -1.0)
			break;
		if(int(boneid[i]) >= MAX_BONES)
		{
			totalposition = vec4(0.0);
			break;
		}
		vec4 localposition = finalbonesmetrices[int(boneid[i])] * vec4(apos, 1.0f);
		totalposition += localposition * weight[i];
	}

	gl_Position = viewproj * model * totalposition;
	fragpos = vec3(model * totalposition);
	Normal = anormal;
	colors = acolors;
}