#version 330 core


const int HORIZONTAL = 0;
const int VERTICAL = 1;

layout (location = 0) out vec4 outputimage;

uniform sampler2D imagetex;

in vec2 texcoords;

uniform int blurtype;

const int blurrange = 9;

void main() {
	
	vec2 unitvec2 = 1.0 / vec2(textureSize(imagetex, 0));
	vec3 total = vec3(0.0);
	int hrange = int(blurrange/2.0);

	for(int i = -hrange; i<=hrange; i++) {
		if(blurtype == VERTICAL) {
			total += texture(imagetex, texcoords + vec2(0.0, unitvec2.y*float(i))).rgb;
		}
		else if(blurtype == HORIZONTAL) {
			total += texture(imagetex, texcoords + vec2(unitvec2.x*float(i), 0.0)).rgb;
		}
	}

	outputimage = vec4(total/blurrange, 1.0);
}