#version 450

#include "common.transcode.glsl"
#include "common.lighting.glsl"


// CONSTANTS
//const vec3 AMBIENT_LIGHT = vec3( 0.03f );
const vec3 AMBIENT_LIGHT = vec3( 0.0002f );
const bool ENABLE_RANGE_FALLOFF = true;

const int LIGHT_COUNT = 3;
const Light lights[LIGHT_COUNT] = Light[](
    Light( vec3( -8.4f, 0.5f, 0.f ), vec3( 200.f, 100.f, 0.f ), .2f, 4.f ),
    Light( vec3( 0.f, 0.5f, 0.f ), vec3( 0.f, 200.f, 0.f ), .5f, 5.f ),
    Light( vec3( 8.4f, 0.5f, 0.f ), vec3( 0.f, 0.f, 200.f ), .2f, 4.f ) );


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

layout ( set = 1, binding = 0 ) uniform sampler shared_sampler;
layout ( set = 1, binding = 2 ) uniform texture2D depth_texture;
layout ( set = 1, binding = 3 ) uniform texture2D albedo_texture;
layout ( set = 1, binding = 4 ) uniform texture2D material_texture;


// FUNCTIONS
vec3 calculate_point_light_irradiance( const Light light, const vec3 world_pos )
{
    const float distance_to_light = length( light.position - world_pos );

    // calculate attenuation
    float range_falloff = 1.f;
    if ( ENABLE_RANGE_FALLOFF )
    {
        range_falloff = pow( clamp( 1.f - distance_to_light / light.range, 0.f, 1.f ), 2.f );
    }
    const float attenuation = range_falloff / max( distance_to_light * distance_to_light, 0.001f );

    // luminous intensity (candela) -> I = Phi / (4 * PI)
    const float I = light.lumen / ( 4.f * PI );

    // spectral illuminance/irradiance -> E = rgb * I * attenuation
    return light.color.rgb * I * attenuation;
}

vec3 calculate_directional_light_irradiance( const Light light, const vec3 world_pos )
{
    // we interpret lumen directly as illuminance (lux) for directional lights.
    const float I = light.lumen;

    // spectral illuminance/irradiance -> E = rgb * I
    // since the light is directional, we don't need to consider attenuation.
    return light.color.rgb * I;
}


// SHADER ENTRY POINT
void main( )
{
    const ivec2 ifrag_coord = ivec2( gl_FragCoord.xy );

    // calculate depth and world position
    const float depth = texelFetch( sampler2D( depth_texture, shared_sampler ), ifrag_coord, 0 ).r;
    const vec3 world_pos = get_world_pos_from_depth( depth, in_uv, mvp.proj, mvp.view );

    // extract material values
    const vec3 albedo = texelFetch( sampler2D( albedo_texture, shared_sampler ), ifrag_coord, 0 ).rgb;
    const float ao = texelFetch( sampler2D( albedo_texture, shared_sampler ), ifrag_coord, 0 ).r;
    const float metallic = texelFetch( sampler2D( material_texture, shared_sampler ), ifrag_coord, 0 ).b;
    const float roughness = texelFetch( sampler2D( material_texture, shared_sampler ), ifrag_coord, 0 ).a;

    // normal and view vector
    const vec3 N = decode16( texelFetch( sampler2D( material_texture, shared_sampler ), ifrag_coord, 0 ).rg );
    const vec3 V = normalize( pc.camera_location - world_pos );

    vec3 F0 = vec3( 0.04f );
    F0 = mix( F0, albedo, metallic );

    // reflectance equation, we calculate per-light cumulative radiance
    vec3 Lo = vec3( 0.f );
    for ( int i = 0; i < LIGHT_COUNT; ++i )
    {
        const vec3 L = normalize( lights[i].position - world_pos );
        const vec3 H = normalize( L + V );

        // irradiance
        const vec3 E = calculate_point_light_irradiance( lights[i], world_pos.xyz );

        // cook-torrance brdf
        const float NDF = distribution_ggx( N, H, roughness );
        const float G = geometry_smith( N, V, L, roughness );
        const vec3 F = fresnel_schlick( max( dot( H, V ), 0.f ), F0 );

        // diffuse and specular components
        const vec3 kS = F;
        const vec3 kD = ( vec3( 1.f ) - kS ) * ( 1.f - metallic );

        const vec3 numerator = NDF * G * F;
        const float denominator = 4.f * max( dot( N, V ), 0.f ) * max( dot( N, L ), 0.f ) + 0.001f;
        const vec3 specular = numerator / denominator;

        // accumulate outgoing radiance
        const float cos_law = max( dot( N, L ), 0.f );
        Lo += ( kD * albedo / PI + specular ) * E * cos_law;
    }

    const vec3 ambient = albedo.rgb * ( AMBIENT_LIGHT * ao );
    const vec3 color = pow( ( ambient + Lo ) / ( ambient + Lo + vec3( 1.f ) ), vec3( 1.f / 2.2f ) );

    out_color = vec4( color.rgb, 1.f );
}