#version 330 core
layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anormal;
layout (location = 2) in vec2 atexcoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 5) in vec4 acolor;
layout (location = 6) in vec4 boneid;
layout (location = 7) in vec4 weight;

out vec3 Normal;

uniform mat4 model, viewproj;

out vec3 fragpos;

out vec2 texcoords;

out vec4 colors;

const int MAX_BONES = 223;
const int MAX_BONE_INFLUENCE = 4;

/*
layout (std140) uniform BoneMetrices {
	mat4 finalbonesmetrices[MAX_BONES];
};
*/
out mat3 tbnmat;
uniform mat4 finalbonesmetrices[MAX_BONES];

uniform bool isAnimated;

void main() {

	mat3 modelmat;

	if(isAnimated) {
		mat4 skinmat = mat4(0.0);
		for(int i = 0; i<MAX_BONE_INFLUENCE; i++) {
			if(int(boneid[i]) == -1.0)
				break;
			if(int(boneid[i]) >= MAX_BONES)
			{
				skinmat = mat4(0.0);
				break;
			}
			skinmat += finalbonesmetrices[int(boneid[i])] * weight[i];
		}
		vec4 totalposition = skinmat * vec4(apos, 1.0);
		gl_Position = viewproj * model * totalposition;
		fragpos = vec3(model * totalposition);
		modelmat = mat3(transpose(inverse(model * skinmat)));
		Normal = modelmat * anormal;
	}
	else {
		gl_Position = viewproj * model * vec4(apos, 1.0);
		fragpos = vec3(model * vec4(apos, 1.0));
		modelmat = mat3(transpose(inverse(model)));
		Normal = modelmat * anormal;
	}

	colors = acolor;

	texcoords = atexcoords;

	vec3 N = normalize(mat3(model) * anormal);
	vec3 T = normalize(mat3(model) * tangent);
	vec3 B = normalize(cross(N, T));

	tbnmat = mat3(T, B, N);
}