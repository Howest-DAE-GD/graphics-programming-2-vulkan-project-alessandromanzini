#version 450

#include "common.transcode.glsl"
#include "common.lighting.glsl"


// CONSTANTS
const int light_count = 3;
const Light lights[light_count] = Light[](
Light( vec3( -5.f, 0.5f, 0.3f ), vec3( 200.f, 0.f, 0.f ), 1.f ),
Light( vec3( 0.f, 0.5f, 0.3f ), vec3( 0.f, 200.f, 0.f ), .4f ),
Light( vec3( 5.f, 0.5f, 0.3f ), vec3( 0.f, 0.f, 200.f ), .8f ) );


// INPUT
layout ( location = 0 ) in vec2 in_uv;


// OUTPUT
layout ( location = 0 ) out vec4 out_color;


// BINDINGS
layout ( push_constant ) uniform CameraPushConstants { vec3 camera_location; } pc;
layout ( set = 0, binding = 0 ) uniform ModelViewProj {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;
layout ( set = 0, binding = 1 ) uniform sampler shared_sampler;
layout ( set = 0, binding = 4 ) uniform texture2D albedo_texture;
layout ( set = 0, binding = 5 ) uniform texture2D material_texture;
layout ( set = 0, binding = 6 ) uniform texture2D depth_texture;


// SHADER ENTRY POINT
void main( )
{
    // calculate depth and world position
    const float depth = texelFetch( sampler2D( depth_texture, shared_sampler ), ivec2( gl_FragCoord.xy ), 0 ).r;
    const vec3 world_pos = get_world_pos_from_depth( depth, in_uv, mvp.proj, mvp.view );

    // extract material values
    const vec3 albedo = texture( sampler2D( albedo_texture, shared_sampler ), in_uv ).rgb;
    const float ao = texture( sampler2D( albedo_texture, shared_sampler ), in_uv ).r;
    const float metallic = texture( sampler2D( material_texture, shared_sampler ), in_uv ).b;
    const float roughness = texture( sampler2D( material_texture, shared_sampler ), in_uv ).a;

    // normal and view vector
    const vec3 N = decode( texture( sampler2D( material_texture, shared_sampler ), in_uv ).rg );
    const vec3 V = normalize( pc.camera_location - world_pos );

    vec3 F0 = vec3( 0.04f );
    F0 = mix( F0, albedo, metallic );

    // reflectance equation
    vec3 Lo = vec3( 0.f );
    for ( int i = 0; i < light_count; ++i )
    {
        // calculate per-light radiance
        const vec3 L = normalize( lights[i].position - world_pos );
        const vec3 H = normalize( L + V );
        const float distance_to_light = length( lights[i].position - world_pos );
        const float attenuation = 1.f / max( distance_to_light * distance_to_light, 0.001f );
        const vec3 I = lights[i].intensity * lights[i].color;
        const vec3 E = I * attenuation;

        // cook-torrance brdf
        const float NDF = distribution_ggx( N, H, roughness );
        const float G = geometry_smith( N, V, L, roughness );
        const vec3 F = fresnel_schlick( max( dot( H, V ), 0.f ), F0 );

        // diffuse and specular components
        const vec3 kS = F;
        vec3 kD = vec3( 1.f ) - kS;
        kD *= 1.f - metallic;

        const vec3 numerator = NDF * G * F;
        const float denominator = 4.f * max( dot( N, V ), 0.f ) * max( dot( N, L ), 0.f ) + 0.001f;
        const vec3 specular = numerator / denominator;

        // accumulate outgoing radiance
        const float cos_law = max( dot( N, L ), 0.f );
        Lo += ( kD * albedo / PI + specular ) * E * cos_law;
    }

    vec3 ambient = vec3( 0.03f ) * albedo * ao;

    vec3 color = ( ambient + Lo );
    color /= ( color + vec3( 1.f ) );
    color = pow( color, vec3( 1.f / 2.2f ) );

    // out_color = vec4( color, 1.f );

    vec3 dir = normalize( world_pos - pc.camera_location );
    out_color = vec4( max( dot( N, dir ), 0.f ) * vec3( 1.f, 1.f, 1.f ), 1.f );

}