#version 330 core

in vec2 texcoords;

out vec4 Fragcolor;

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



uniform sampler2D positionbuffer;
uniform sampler2D normalbuffer;
uniform sampler2D albedobuffer;

uniform mat4 cammatrix[16];

uniform Light lights[16];

uniform sampler2D textureshadowmap[8];
uniform samplerCube cubeshadowmap[8];


uniform bool hastexture;

uniform vec3 camerapos;



float getShadowFactor(int index, vec3 fragpos, vec3 normal) {
    
   
    if (lights[index].index == -1) return 1.0; 
    
  
    float shadowFactor = 0.0;
    
    // --- Point Light Shadow Logic (Cubemap) ---
    if (lights[index].type == POINT_LIGHT) {
        
        // **TODO: Pass far plane as a uniform or struct member, 99.0 is a hardcoded guess**
        float farplane = 100.0; 
        
        vec3 lightToFrag = fragpos - lights[index].position;
        float currentDepth = length(lightToFrag) / farplane;
        vec3 dir = normalize(lightToFrag);
        //dir.y = -dir.y;

        // Simple single sample for closest depth (no PCF on cubemap in this simple setup)
        float closestDepth = texture(cubeshadowmap[index], dir).r;
        
        // Bias calculation
        // NOTE: Directional bias calculation (NdotL) is often less effective for point lights
        // float NdotL = max(dot(normal, normalize(lightToFrag)), 0.0); // Use light direction toward fragment
        // float bias = max(0.005 * (1.0 - NdotL), 0.01);
        float bias = 0.005; // Use a constant bias for simplicity or fine-tune one based on NdotL
        
        // Shadow comparison
        if (currentDepth - bias > closestDepth) {
            shadowFactor = 0.0; // Shadowed
        } else {
            shadowFactor = 1.0; // Not shadowed
        }

    } 
    // --- Directional / Spot Light Shadow Logic (Planar Shadow Map with PCF) ---
    else { 
        
        int radius = 3;
        int nh = int(radius / 2);
        float totalSamples = float(radius * radius);
        float illuminatedSamples = 0.0;

        for(int i = -nh; i <= nh; i++) {
            for(int j = -nh; j <= nh; j++) {
                
                vec3 newpos = fragpos; 
                vec4 lightClipPos = cammatrix[index] * vec4(newpos, 1.0);
                vec3 projCoords = lightClipPos.xyz / lightClipPos.w;
                vec2 texCoords = projCoords.xy * 0.5 + 0.5;

                // PCF offset using textureSize
                texCoords += vec2(i, j) / vec2(textureSize(textureshadowmap[index], 0));

                float closestDepth = texture(textureshadowmap[index], texCoords).r;
                float currentDepth = projCoords.z * 0.5 + 0.5;

                // Bias calculation
                // For directional/spot lights, -lights[index].direction is the light vector.
                float NdotL = max(dot(normal, -lights[index].direction), 0.0);
                float bias = max(0.005 * (1.0 - NdotL), 0.01);
                
                // Shadow comparison
                if (currentDepth - bias > closestDepth) {
                
                } else {
                    illuminatedSamples += 1.0;
                }
            }
        }
        
        shadowFactor = illuminatedSamples / totalSamples;
    }
  
    return shadowFactor;
}


void main() {
	vec3 color = vec3(0.0);

    vec3 Normal = (texture(normalbuffer, texcoords).xyz * 2.0) - 1.0;
	vec3 N = normalize(Normal);

    vec3 fragpos = texture(positionbuffer, texcoords).xyz;

	int index = 0;
    while(lights[index].index != -1) {

       
		vec3 dis = lights[index].position - fragpos;
		float distance = length(dis);

        vec3 nd = normalize(lights[index].direction);
        
 
        float attenuation = 1.0;
        if(lights[index].type == SPOT_LIGHT || lights[index].type == POINT_LIGHT) {
            distance /= 30.0;
            attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        }

       
		float diffuse = 0.0;

		if(lights[index].type == DIRECTIONAL_LIGHT) {
			diffuse = max(dot(-nd, N), 0.0);
		}
		else if(lights[index].type == SPOT_LIGHT) {
			float a = max(dot(normalize(-dis), nd), 0.0);
			float maxlight = cos(3.14159 * (lights[index].degree / 180.0));
			
			if(a < maxlight) 
				a = 0;
			
			diffuse = max(dot(normalize(dis), N), 0.0) * a;
		}
		else if(lights[index].type == POINT_LIGHT) {
			diffuse = max(dot(normalize(dis), normalize(Normal)), 0.0);
		}


        
        vec3 viewdir = camerapos - fragpos;
        vec3 lightdir = lights[index].position - fragpos;
        vec3 reflectdir = reflect(normalize(-lightdir), N);
        float specular = pow(max(dot(reflectdir, normalize(viewdir)), 0.0), 32);

        if(lights[index].type == SPOT_LIGHT) {
            float a = max(dot(normalize(-dis), nd), 0.0);
			float maxlight = cos(3.14159 * (lights[index].degree / 180.0));
			
			if(a < maxlight) 
				specular = 0.0;
        }

        vec3 amb = lights[index].color * 0.1;
        float shadowFactor = getShadowFactor(index, fragpos, N);
        if(shadowFactor < 0.3)
            specular = 0.0;

        vec3 diffuseterm = lights[index].color * diffuse;
        vec3 specularterm = lights[index].color * specular;

        color += amb + ((diffuseterm + specularterm) * shadowFactor * attenuation);


		index++;
	}

	Fragcolor = texture(albedobuffer, texcoords) * vec4(color, 1.0);
}