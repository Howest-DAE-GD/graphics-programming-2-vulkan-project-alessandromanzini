#version 450

#include "common.transcode.glsl"
#include "common.lighting.glsl"


// CONSTANTS
const float EXPOSURE_COMPENSATION = 0.35f;
const bool ENABLE_RANGE_FALLOFF = true;

const int LIGHT_COUNT = 3;
const Light lights[LIGHT_COUNT] = Light[](
    //Light( vec3( 1.f, 0.f, 0.f ), vec3( 200.f, 100.f, 0.f ), 10.f, 4.f, 1 ),
    Light( vec3( -8.4f, 0.5f, 0.f ), vec3( 200.f, 100.f, 0.f ), 10.f, 4.f, 0 ),
    Light( vec3( 0.f, 0.5f, 0.f ), vec3( 0.f, 200.f, 0.f ), .5f, 5.f, 0 ),
    Light( vec3( 8.4f, 0.5f, 0.f ), vec3( 0.f, 0.f, 200.f ), .2f, 4.f, 0 ) );


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
layout ( set = 2, binding = 2 ) uniform textureCube environment_map;
layout ( set = 2, binding = 3 ) uniform textureCube diffuse_irradiance_map;


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

void calculate_direct_diffuse_specular(
in vec3 N, in vec3 V, in vec3 L, in vec3 H, in vec3 albedo, in float metallic, in float roughness, in vec3 F0, out vec3 diffuse, out vec3 specular )
{
    // cook-torrance brdf
    const float NDF = distribution_ggx( N, H, roughness );
    const float G = geometry_smith( N, V, L, roughness, false );
    const vec3 F = fresnel_schlick( max( dot( H, V ), 0.f ), F0 );

    // diffuse and specular components
    const vec3 kS = F;
    const vec3 kD = ( vec3( 1.f ) - kS ) * ( 1.f - metallic );

    const vec3 numerator = NDF * G * F;
    const float denominator = 4.f * max( dot( N, V ), 0.f ) * max( dot( N, L ), 0.f ) + 0.001f;
    specular = numerator / denominator;
    diffuse = kD * albedo.rgb / PI;
}

vec3 calculate_ambient_light( in vec3 N, in vec3 V, in vec3 albedo, in float metallic, in float roughness, in vec3 F0 )
{
    const vec3 F = fresnel_schlick_roughness( max( dot( V, N ), 0.f ), F0, roughness );
    const vec3 prefiltered_diffuse_E = texture( samplerCube( diffuse_irradiance_map, shared_sampler ), vec3( N.x, -N.y, N.z ) ).rgb;
    const vec3 kD = ( 1.f - F ) * ( 1.f - metallic );
    return kD * prefiltered_diffuse_E * albedo.rgb;
}


// SHADER ENTRY POINT
void main( )
{
    const ivec2 ifrag_coord = ivec2( gl_FragCoord.xy );

    // calculate depth and world position
    const float depth = texelFetch( sampler2D( depth_texture, shared_sampler ), ifrag_coord, 0 ).r;
    const vec3 world_pos = get_world_pos_from_depth( depth, in_uv, mvp.proj, mvp.view );

    // if we are outside the view frustum, we sample from the environment map and skip lighting
    if ( depth >= 1.f )
    {
        const vec3 sample_direction = normalize( world_pos.xyz );
        const vec3 color = texture( samplerCube( environment_map, shared_sampler ), sample_direction ).rgb;
        out_color = vec4( color, 1.f );
        return;
    }

    // fetch material values
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
        vec3 E; vec3 L;
        switch ( lights[i].type )
        {
            case 0: // point light
                E = calculate_point_light_irradiance( lights[i], world_pos.xyz );
                L = normalize( lights[i].position - world_pos );
                break;

            case 1: // directional light
                E = calculate_directional_light_irradiance( lights[i], world_pos.xyz );
                L = normalize( lights[i].position );
                break;

            default: // unsupported light type
                continue;
        }

        const vec3 H = normalize( L + V );

        // diffuse and specular components
        vec3 diffuse; vec3 specular;
        calculate_direct_diffuse_specular( N, V, L, H, albedo, metallic, roughness, F0, diffuse, specular );

        // lambertian cosine law
        const float cos_law = max( dot( N, L ), 0.f );

        // accumulate outgoing radiance
        Lo += ( diffuse + specular ) * E * cos_law;
    }

    // calculate indirect irradiance
    const vec3 ambient = calculate_ambient_light( N, V, albedo, metallic, roughness, F0 );

    vec3 final_color = ( Lo + ambient ) * ao * EXPOSURE_COMPENSATION;
    final_color = pow( final_color / ( final_color + vec3( 1.f ) ), vec3( 1.f / 2.2f ) );

    out_color = vec4( final_color.rgb, 1.f );
}