#version 330 core

const int HORIZONTAL = 0;
const int VERTICAL = 1;

in vec2 texcoords;

uniform sampler2D inputimage;

layout (location = 0) out float outputimage;

uniform int blurtype;

const int blurrange = 9;

void main() {

	int hrange = int(blurrange/2.0);
	float total = 0.0;
	for(int i = -hrange; i<=hrange; i++) {
		vec2 newtexcoords = texcoords;
		if(blurtype == HORIZONTAL) {
			newtexcoords += vec2(0.0, i) / vec2(textureSize(inputimage, 0));
		}
		else if(blurtype == VERTICAL) {
			newtexcoords += vec2(i, 0.0) / vec2(textureSize(inputimage, 0));
		}
		total += texture(inputimage, newtexcoords).r;
	}
	total /= blurrange;

	outputimage = total;
}