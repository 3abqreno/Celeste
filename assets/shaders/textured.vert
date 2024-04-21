#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;

out Varyings {
    vec4 color;
    vec2 tex_coord;
} vs_out;

uniform mat4 transform;

void main(){
    //TODO: (Req 7) Change the next line to apply the transformation matrix
<<<<<<< HEAD
    gl_Position = transform * vec4(position, 1.0);
=======
    gl_Position = vec4(position, 1.0);
>>>>>>> d5e7fd6c6bbf01183d6026238af8e508ea95a91c
    vs_out.color = color;
    vs_out.tex_coord = tex_coord;
}