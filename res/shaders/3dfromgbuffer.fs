#version 330 core

in vec2 texcoords;

layout (location = 0) out vec4 Fragcolor;

const int DIRECTIONAL_LIGHT = 0;
const int SPOT_LIGHT = 1;
const int POINT_LIGHT = 2;
const int NO_SHADOW_POINT_LIGHT = 3;

const int occlusioncount = 32;

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
uniform sampler2D depthbuffer;
uniform sampler2D ssaobuffer;

uniform mat4 cammatrix[16];

uniform Light lights[16];

uniform sampler2D textureshadowmap[8];
uniform samplerCube cubeshadowmap[8];


uniform bool hastexture;

uniform vec3 camerapos;

uniform mat4 viewproj;

uniform float ambscale;

float getShadowFactor(int index, vec3 fragpos, vec3 normal) {
    
   
    if (lights[index].index == -1) return 1.0; 
    
  
    float shadowFactor = 0.0;
    
    if(lights[index].type == NO_SHADOW_POINT_LIGHT) {
        return 1.0;
    }
    // --- Point Light Shadow Logic (Cubemap) ---
    else if (lights[index].type == POINT_LIGHT) {
        
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
        vec3 L = normalize(dis);                    // <-- FIX: Define L (light direction)
        float NdotL = max(dot(N, L), 0.0);
        vec3 nd = normalize(lights[index].direction);
        
 
        float attenuation = 1.0;
        if(lights[index].type == SPOT_LIGHT || lights[index].type == POINT_LIGHT|| lights[index].type == NO_SHADOW_POINT_LIGHT) {
            distance /= 30.0;
            attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        }

       
		float diffuse = 0.0;

		if(lights[index].type == DIRECTIONAL_LIGHT) {
			diffuse = max(dot(-nd, N), 0.0);
		}
		else if(lights[index].type == SPOT_LIGHT) {
		

            diffuse = NdotL;
    
            vec3 spotDir = nd;
    
           
            float cosTheta = dot(-L, spotDir);

            
            float cosInnerCutoff = cos(radians(lights[index].degree));
            float cosOuterCutoff = cos(radians(lights[index].degree + 5.0));
            
            float spotFactor = smoothstep(cosOuterCutoff, 
                                        cosInnerCutoff, 
                                        cosTheta);
    
            
            diffuse = NdotL * spotFactor;
        }
		
		else if(lights[index].type == POINT_LIGHT || lights[index].type == NO_SHADOW_POINT_LIGHT) {
			diffuse = max(dot(normalize(dis), normalize(Normal)), 0.0);
		}



        
        
        vec3 viewdir = camerapos - fragpos;
        vec3 lightdir = lights[index].position - fragpos;
        vec3 reflectdir = reflect(normalize(-lightdir), N);
        float specular = pow(max(dot(reflectdir, normalize(viewdir)), 0.0), 32);

        
        if(diffuse == 0)
            specular = 0;



        float shadowFactor = getShadowFactor(index, fragpos, N);
        if(shadowFactor < 0.3)
            specular = 0.0;


        
        vec3 diffuseterm = lights[index].color * diffuse;
        vec3 specularterm = lights[index].color * specular;


        color += (diffuseterm + specularterm) * shadowFactor * attenuation;


		index++;
	}

    vec3 amb = vec3(ambscale);
    float ssao = texture(ssaobuffer, texcoords).r;
    color += (amb * ssao);

	Fragcolor = texture(albedobuffer, texcoords) * vec4(color, 1.0);

}