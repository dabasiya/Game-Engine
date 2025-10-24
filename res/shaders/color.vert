#version 330 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec4 acolor;
layout (location = 2) in float atexindex;
layout (location = 3) in vec2 atexcoords;

uniform mat4 projectionview;


out float texindex;
out vec4 color;
out vec2 texcoords;

void main() {
    gl_Position = projectionview * vec4(apos, 1.0);
    color = acolor;
    texcoords = atexcoords;
    texindex = atexindex;
}