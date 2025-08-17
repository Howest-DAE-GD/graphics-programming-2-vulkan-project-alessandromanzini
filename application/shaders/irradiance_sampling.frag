#version 450

#include "common.math.glsl"


// OUTPUT
layout ( location = 0 ) out vec4 out_color;


// INPUT
layout ( location = 0 ) in vec3 in_local_position;


// BINDING
layout ( set = 0, binding = 0 ) uniform sampler shared_sampler;
layout ( set = 0, binding = 2 ) uniform textureCube environment_map;


// SHADER ENTRY POINT
void main( )
{
    vec3 N = normalize( in_local_position );
    vec3 tangent, bitangent;
    tangent_to_world( N, tangent, bitangent );

    // original
    vec3 E = vec3( 0.f );
    float sample_count = 0.f;
    const float sample_delta = 0.025f;
    for ( float phi = 0.f; phi < 2.f * PI; phi += sample_delta )
    {
        for ( float theta = 0.f; theta < 0.5f * PI; theta += sample_delta )
        {
            // spherical coordinates to cartesian ( tangent space )
            vec3 tangent_sample = vec3( sin( theta ) * cos( phi ), sin( theta ) * sin( phi ), cos( theta ) );

            // tangent space to world space
            vec3 sample_vec = tangent_sample.x * tangent + tangent_sample.y * bitangent + tangent_sample.z * N;

            // sample and accumulate
            const vec3 sample_direction = normalize( vec3( sample_spherical_map( sample_vec ), 0.f ) );
            E += texture( samplerCube( environment_map, shared_sampler ), sample_direction ).rgb * cos( theta ) * sin( theta );
            ++sample_count;
        }
    }

    // integrate and store
    E = PI * E * ( 1.f / float( sample_count ) );
    out_color = vec4( E.rgb, 1.f );
}