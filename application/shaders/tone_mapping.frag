#version 450

#include "common.exposure.glsl"
#include "common.tone.glsl"


// BINDINGS
layout ( set = 1, binding = 0 ) uniform sampler shared_sampler;
layout ( set = 1, binding = 5 ) uniform texture2D hdr_color_texture;


// OUTPUT
layout ( location = 0 ) out vec4 out_color;


// SHADER ENTRY POINT
void main( )
{
    const ivec2 ifrag_coord = ivec2( gl_FragCoord.xy );
    const vec3 hdr_color = texelFetch( sampler2D( hdr_color_texture, shared_sampler ), ifrag_coord, 0 ).rgb;

    const float EV100 = 1.4f;
    const float exposure = EV100_to_exposure( EV100, 1.2f );

    out_color = vec4( ACES_film_tone_mapping( hdr_color.rgb * exposure ).rgb, 1.f );
}