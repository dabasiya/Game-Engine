#version 330 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec4 acolor;
layout (location = 2) in float atexindex;
layout (location = 3) in vec2 atexcoords;
layout (location = 4) in float entityid;

uniform mat4 projectionview;


out vec4 color;
out float texindex;
out vec2 texcoords;

void main() {
    int entid = int(entityid);
    gl_Position = projectionview * vec4(apos, 1.0);
    vec3 tcolor = vec3(0.0);
    tcolor.r = (entid & 0xFF) / 255.0f;
    tcolor.g = ((entid >> 8) & 0xFF) / 255.0f;
    tcolor.b = ((entid >> 16) & 0xFF) / 255.0f;
    color = vec4(tcolor, 1.0f);
    texindex = atexindex;
    texcoords = atexcoords;
}