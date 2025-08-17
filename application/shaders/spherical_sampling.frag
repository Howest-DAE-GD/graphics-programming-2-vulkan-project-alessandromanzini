#version 450

#include "common.math.glsl"


// OUTPUT
layout ( location = 0 ) out vec4 out_color;


// INPUT
layout ( location = 0 ) in vec3 in_local_position;


// BINDING
layout ( set = 0, binding = 0 ) uniform sampler hdri_sampler;
layout ( set = 0, binding = 1 ) uniform texture2D hdri_image;


// SHADER ENTRY POINT
void main( )
{
    vec3 direction = normalize( in_local_position );
    direction = vec3( direction.z, direction.y, -direction.x );

    out_color = vec4( texture( sampler2D( hdri_image, hdri_sampler ), sample_spherical_map( direction ) ).rgb, 1.f );
}