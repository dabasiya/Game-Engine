#version 330 core

in vec2 texcoords;

out vec4 Fragcolor;

uniform sampler2D teximage;
uniform sampler2D bloomimage;
uniform sampler2D ssaoimage;
uniform sampler2D depthimage;

const int FINAL_OUTPUT = 0;
const int SSAO = 1;
const int DEPTH = 2;

uniform int renderoutput;


void main() {

	if(renderoutput == FINAL_OUTPUT) {
		vec4 bloomcolor = texture(bloomimage, texcoords);
		Fragcolor = (texture(teximage, texcoords) + bloomcolor);
	}

	else if(renderoutput == SSAO) {
		float r = texture(ssaoimage, texcoords).r;
		Fragcolor = vec4(r, r, r, 1.0);
	}

	else if(renderoutput == DEPTH) {
		float r = texture(depthimage, texcoords).r;
		Fragcolor = vec4(r, r, r, 1.0);
	}
}