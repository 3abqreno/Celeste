#version 330 core

// This shader is designed to work with "triangle.vert" and it receives an
// interpolated varying which represents the vertex color.

in Varyings {
    vec3 color;
} fs_in;

out vec4 frag_color;

// Define uniform variables for the color matrices: red, green, and blue
uniform vec4 red = vec4(1.0, 0.0, 0.0, 0.0);
uniform vec4 green = vec4(0.0, 1.0, 0.0, 0.0);
uniform vec4 blue = vec4(0.0, 0.0, 1.0, 0.0);

void main() {
    // Apply channel mixing using dot product
    vec3 mixedColor = vec3(
        dot(red, vec4(fs_in.color, 1.0)),
        dot(green, vec4(fs_in.color, 1.0)),
        dot(blue, vec4(fs_in.color, 1.0))
    );

    // Output mixed color
    frag_color = vec4(mixedColor, 1.0);
}
