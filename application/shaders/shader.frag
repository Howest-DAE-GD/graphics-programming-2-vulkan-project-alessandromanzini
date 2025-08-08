#version 450

// INPUT
layout ( location = 0 ) in vec3 fragColor;
layout ( location = 1 ) in vec2 fragTexCoord;

// OUTPUT
layout ( location = 0 ) out vec4 outColor;

// BINDINGS
layout ( binding = 1 ) uniform sampler2D texSampler;

// The triangle that is formed by the positions from the vertex shader fills an area on the screen with fragments.
// The fragment shader is invoked on these fragments to produce a color and depth for the frame buffer.
// The main function is called for every fragment and will provide automatic fragment interpolation.
void main( )
{
    outColor = vec4( fragColor * texture( texSampler, fragTexCoord ).rgb, 1.0 );
}