#version 330 core

const int DIRECTIONAL_LIGHT = 0;
const int SPOT_LIGHT = 1;
const int POINT_LIGHT = 2;

struct Light {
	vec3 position;
	int type;
	vec3 direction;
	vec3 color;
	float degree;
	int index;
};

out vec4 Fragcolor;

in vec3 fragpos;

in vec2 texcoords;

in vec4 colors;

in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform mat4 cammatrix[16];

uniform Light lights[16];

uniform sampler2D textureshadowmap[16];
uniform samplerCube cubeshadowmap[16];

uniform bool hastexture;


float getavarageshadow(vec3 pos) {
	
	int index = 0;

	float shadow = 0;
	while(lights[index].index != -1) {

		if(lights[index].type == DIRECTIONAL_LIGHT || lights[index].type == SPOT_LIGHT) {

			int radius = 3;
			int nh = int(radius/2);
			for(int i = -nh; i<nh+1; i++) {
				for(int j = -nh; j<nh+1; j++) {
					vec3 newpos = vec3(pos.x + i/1024.0, pos.y + j/1024.0, pos.z);
					vec4 texcoor = cammatrix[index] * vec4(newpos, 1.0);
					texcoor.xyz /= texcoor.w;
					texcoor = texcoor * 0.5 + 0.5;

					float a = texture(textureshadowmap[index], texcoor.xy).r;
			
					float b = texcoor.z;
					float bias = max(0.0005 * (1.0 - dot(Normal, -lights[index].direction)), 0.005);
					if(b - bias > a) shadow += 1.0;
				}
			}

			shadow /= 9;
		}

		/*
		else if(lights[index].type == POINT_LIGHT) {
			vec3 fragtolight = fragpos - lights[index].position;

			float a = texture(cubeshadowmap[index], normalize(fragtolight)).r;
			
			float b = length(fragtolight);
			a *= 10.0;

			float bias = max(0.05 * (1.0 - dot(Normal, fragtolight)), 0.005);
			if(b - bias > a) shadow += 1.0;
		}

		*/
		index++;
	}

	if(index > 0)
		shadow /= float(index);

	return shadow;
}

void main() {
	vec3 color = vec3(0.0);

	int index = 0;
	while(lights[index].index != -1) {

		vec3 dis = lights[index].position - fragpos;

		float diffuse = 0.0;

		if(lights[index].type == DIRECTIONAL_LIGHT) {
			diffuse = max(dot(-lights[index].direction, normalize(Normal)), 0.0);
		}
		else if(lights[index].type == SPOT_LIGHT) {
			float a = max(dot(normalize(dis), normalize(-lights[index].direction)), 0.0);
			float maxlight = cos(3.14159 * (lights[index].degree / 180.0));
			
			if(a < maxlight) 
				a = 0;
			
			diffuse = max(dot(normalize(dis), normalize(Normal)), 0.0) * a;
		}
		else if(lights[index].type == POINT_LIGHT) {
			diffuse = max(dot(normalize(dis), normalize(Normal)), 0.0);
		}

		float distance = length(dis);

		float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
		vec3 ambient = 0.1 * lights[index].color;
        color += ambient + (lights[index].color * diffuse * attenuation);

		index++;
	}
	
	if(index > 0) 
		color /= index;

	color *= (1 - getavarageshadow(fragpos));

	if(hastexture)
		Fragcolor = texture(texture_diffuse1, texcoords) * vec4(color, 1.0);
	else 
		Fragcolor = colors * vec4(color, 1.0); 
}