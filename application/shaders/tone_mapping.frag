#version 450

#include "common.exposure.glsl"
#include "common.tone.glsl"


// BINDINGS
layout ( set = 1, binding = 0 ) uniform sampler shared_sampler;
layout ( set = 1, binding = 5 ) uniform texture2D hdr_color_texture;


// INPUT
layout ( location = 0 ) in vec2 in_uv;


// OUTPUT
layout ( location = 0 ) out vec4 out_color;


// SHADER ENTRY POINT
void main( )
{
    const vec3 hdr_color = texture( sampler2D( hdr_color_texture, shared_sampler ), in_uv ).rgb;

    const float EV100 = 0.f;
    const float exposure = EV100_to_exposure( EV100, 1.2f );

    out_color = vec4( uncharted2_tone_mapping( hdr_color.rgb * exposure ).rgb, 1.f );
}