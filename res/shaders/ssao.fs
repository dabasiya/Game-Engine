#version 330 core

in vec2 texcoords;

uniform sampler2D positionbuffer;
uniform sampler2D normalbuffer;
uniform sampler2D depthbuffer;

uniform mat4 viewproj;

const int totalsamples = 16;

uniform float camerarange;

layout (location = 0) out float ssaobuffer;


float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453123);
}

float rand(vec2 uv) {
    return hash(uv);
}

vec3 randomHemisphereDirection(float x1, float x2) {
    
    float phi = 2.0 * 3.14 * x1;
    float cosTheta = x2;

    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
    return vec3(cos(phi) * sinTheta, sin(phi)*sinTheta, cosTheta);
}

void main() {
	
        vec3 fragpos = texture(positionbuffer, texcoords).xyz;
        vec3 Normal = (texture(normalbuffer, texcoords).xyz * 2.0) - 1.0;
	    vec3 N = normalize(Normal);

        // ssao

        vec3 T = vec3(N.z, N.x, N.y);
        vec3 B = cross(T, N);

        mat3 TBN = mat3(T, B, N);

        
        vec3 samplevectors[totalsamples];
        for(int i = 0; i<totalsamples; i++) {
            vec3 sample = randomHemisphereDirection(rand(gl_FragCoord.xy), rand(gl_FragCoord.xy));
            float scale = float(i)/totalsamples;
            scale = mix(0.1, 1.0, scale*scale);
            sample *= scale;
            samplevectors[i] = (TBN * sample);
        }

        float ao = 0.0;
        
        for(int i = 0; i<totalsamples; i++) {
            vec3 newpos = fragpos + samplevectors[i];
            vec4 temppos = viewproj * vec4(newpos, 1.0);
            vec3 ntemppos = temppos.xyz / temppos.w;
            ntemppos = ntemppos / 2.0 + 0.5;
            float closestDepth = texture(depthbuffer, ntemppos.xy).r;
            float currentDepth = ntemppos.z;

            if(currentDepth > closestDepth) {
                currentDepth *= camerarange;
                closestDepth *= camerarange;

                if(currentDepth - closestDepth < 0.1)
                    ao += 1.0;
            }
        }

        ao = (1.0 - ao/totalsamples);

        ssaobuffer = ao;
}