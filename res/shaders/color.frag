#version 330 core

out vec4 fragcolor;

in vec4 color;
in vec2 texcoords;
in float texindex;

uniform sampler2D tex[16];


void main() {
     int tindex = int(texindex);
     vec4 fragc = color * texture(tex[tindex], texcoords);
     if(fragc.a == 0.0)
        discard;
     fragcolor = fragc;
}