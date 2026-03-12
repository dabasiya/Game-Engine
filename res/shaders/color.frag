#version 330 core

out vec4 fragcolor;

in vec4 color;
in vec2 texcoords;
in float texindex;

uniform sampler2D tex[16];

uniform bool isui;

flat in vec4 cliprect;
in vec2 pos;


void main() {
     int tindex = int(texindex);
     vec4 fragc = color * texture(tex[tindex], texcoords);
     

     if(isui) {

        if((pos.x > cliprect.x && pos.y > cliprect.y) && (pos.x < cliprect.z && pos.y < cliprect.w))
            fragcolor = fragc;
        else 
            discard;
    }

    fragcolor = fragc;
}