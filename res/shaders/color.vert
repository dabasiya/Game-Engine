#version 330 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec4 acolor;
layout (location = 2) in float atexindex;
layout (location = 3) in vec2 atexcoords;

uniform mat4 projectionview;

uniform vec4 clips[16];
uniform int quadindexes[16];

uniform bool isui;

flat out vec4 cliprect;
out vec2 pos;

out float texindex;
out vec4 color;
out vec2 texcoords;

void main() {
    gl_Position = projectionview * vec4(apos, 1.0);
    color = acolor;
    texcoords = atexcoords;
    texindex = atexindex;

    if(isui) {
        pos = vec2(apos.x, apos.y);
        int elementid = gl_VertexID / 4;
        for(int i = 0; i<16; i++) {
            
            if(elementid >= quadindexes[i+1])
                continue;
            else if(elementid >= quadindexes[i])
            {
                cliprect = clips[i];
                break;
            }
        }
    }
}