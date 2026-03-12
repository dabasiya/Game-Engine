#version 330 core
layout (location = 0) in vec3 apos;
layout (location = 6) in vec4 boneid;
layout (location = 7) in vec4 weight;

uniform mat4 model, viewproj;

const int MAX_BONES = 223;
const int MAX_BONE_INFLUENCE = 4;

/*
layout (std140) uniform BoneMetrices {
	mat4 finalbonesmetrices[MAX_BONES];
};
*/

uniform mat4 finalbonesmetrices[MAX_BONES];

uniform bool isAnimated;


void main() {

	if(isAnimated) {
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
	}
	else {
		gl_Position = viewproj * model * vec4(apos, 1.0);
	}
}