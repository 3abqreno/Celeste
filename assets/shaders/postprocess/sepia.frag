#version 330

// The texture holding the scene pixels
uniform sampler2D tex;

// Read "assets/shaders/fullscreen.vert" to know what "tex_coord" holds;
in vec2 tex_coord;
out vec4 frag_color;

// How far (in the texture space) is the distance (on the x-axis) between
// the pixels from which the red/green (or green/blue) channels are sampled
#define STRENGTH 0.005

// Chromatic aberration mimics some old cameras where the lens disperses light
// differently based on its wavelength. In this shader, we will implement a
// cheap version of that effect 

void main(){
    vec3 color = texture(tex, tex_coord).rgb;
    
    // Calculate luminance
    float gray = dot(color, vec3(0.3, 0.59, 0.11));
    
    // Create sepia colors by linearly transforming the grayscale
    vec3 sepia;
    sepia.r = gray + 0.9 * (0.9 - gray);  // Red channel
    sepia.g = gray + 0.5 * (0.76 - gray); // Green channel
    sepia.b = gray + 0.3 * (0.45 - gray); // Blue channel

    // Clamp values to ensure they are within a valid range
    sepia = clamp(sepia, 0.0, 1.0);

    // Output the color
    frag_color = vec4(sepia, 1.0);
}