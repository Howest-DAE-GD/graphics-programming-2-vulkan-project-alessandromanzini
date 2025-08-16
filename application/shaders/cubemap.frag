#version 450


// CONSTANTS
const vec2 INV_TAN = vec2( 0.1591f, 0.3183f ); // 1/(2π), 1/π


// OUTPUT
layout ( location = 0 ) out vec4 out_color;


// INPUT
layout ( location = 0 ) in vec3 in_local_position;


// BINDING
layout ( set = 1, binding = 0 ) uniform sampler hdri_sampler;
layout ( set = 2, binding = 0 ) uniform texture2D hdri_image;


// FUNCTIONS
vec2 sample_spherical_map( in vec3 v )
{
    vec2 uv = vec2( atan( v.z, v.x ), asin( v.y ) );
    uv *= INV_TAN;
    uv += 0.5f;
    return uv;
}


// SHADER ENTRY POINT
void main( )
{
    vec3 direction = normalize( in_local_position );
     direction = vec3( direction.z, direction.y, -direction.x );

    out_color = vec4( texture( sampler2D( hdri_image, hdri_sampler ), sample_spherical_map( direction ) ).rgb, 1.f );
}