#version 330 core

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

layout (location = 0) out vec3 PositionBuffer;
layout (location = 1) out vec3 NormalBuffer;
layout (location = 2) out vec4 Albedo;
layout (location = 3) out vec4 bloombuffer;

in vec3 fragpos;

in vec3 Normal;

in vec2 texcoords;

in vec4 colors;

in mat3 tbnmat;

uniform bool bloom;

uniform bool hastexture;
uniform bool hasnormalmap;

void main() {
	PositionBuffer = fragpos;

	if(hasnormalmap) {
		vec3 normal = texture(texture_normal1, texcoords).rgb;
        normal = normalize(normal * 2.0 - 1.0);  // Convert [0,1] -> [-1,1]
        NormalBuffer = normalize(tbnmat * normal);
		NormalBuffer = NormalBuffer * 0.5 + 0.5;
	}
	else {
		NormalBuffer = normalize(Normal);
		NormalBuffer = NormalBuffer * 0.5 + 0.5; // Map from [-1,1] to [0,1]
	}

	if(hastexture)
		Albedo = texture(texture_diffuse1, texcoords);
	else
		Albedo = colors;

	if(bloom)
		bloombuffer = Albedo;
	else 
		bloombuffer = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}