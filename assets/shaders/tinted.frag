#version 330 core

in Varyings {
    vec4 color;
} fs_in;

out vec4 frag_color;

uniform vec4 tint;

void main(){
    //TODO: (Req 7) Modify the following line to compute the fragment color
    // by multiplying the tint with the vertex color
<<<<<<< HEAD
    frag_color = fs_in.color * tint;
=======
    frag_color = vec4(1.0);
>>>>>>> d5e7fd6c6bbf01183d6026238af8e508ea95a91c
}