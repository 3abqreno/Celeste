#version 330

// This vertex shader should be used to render a triangle whose normalized device coordinates are:
// (-0.5, -0.5, 0.0), ( 0.5, -0.5, 0.0), ( 0.0,  0.5, 0.0)
// And it also should send the vertex color as a varying to the fragment shader where the colors are (in order):
// (1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0)

out Varyings {
    vec3 color;
} vs_out;

// Declare uniform variables for translation and scale
uniform vec2 translation = vec2(0.0, 0.0);
uniform vec2 scale = vec2(1.0, 1.0);

void main() {
    // Define the positions of the triangle vertices
    vec3 vertices[3] = vec3[3](
        vec3(-0.5, -0.5, 0.0),
        vec3(0.5, -0.5, 0.0),
        vec3(0.0, 0.5, 0.0)
    );
    
    // Transform vertex position based on scale and translation
    vec3 vertex = vertices[gl_VertexID];
    vertex = vec3(scale.x * vertex.x + translation.x, scale.y * vertex.y + translation.y, vertex.z);

    // Set vertex color based on gl_VertexID
    vs_out.color = vec3(
        gl_VertexID == 0 ? 1.0 : 0.0,
        gl_VertexID == 1 ? 1.0 : 0.0,
        gl_VertexID == 2 ? 1.0 : 0.0
    );

    // Output vertex position and color
    gl_Position = vec4(vertex, 1.0);
}
