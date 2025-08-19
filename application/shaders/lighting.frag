#version 450
#extension GL_EXT_samplerless_texture_functions: enable

#include "common.transcode.glsl"
#include "common.lighting.glsl"


// CONSTANTS
const float EXPOSURE_COMPENSATION = 0.6f;
const bool ENABLE_RANGE_FALLOFF = true;


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

layout ( constant_id = 0 ) const uint LIGHT_COUNT = 1u;
layout ( set = 0, binding = 2 ) uniform LightBufferData { Light lights[LIGHT_COUNT]; } light_buffer;
layout ( set = 3, binding = 0 ) uniform sampler shadow_sampler;
layout ( set = 3, binding = 1 ) uniform texture2D shadow_map_texures[LIGHT_COUNT];


// FUNCTIONS
vec3 calculate_point_light_irradiance( in const Light light, in const vec3 world_pos )
{
    const float distance_to_light = length( light.position.xyz - world_pos );

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
    return clamp( kelvin_to_rgb( light.kelvin ), 0.f, 1.f ) * I * attenuation;
}


vec3 calculate_directional_light_irradiance( in const Light light, in const vec3 world_pos )
{
    // we interpret lumen directly as illuminance (lux) for directional lights.
    const float I = light.lumen;

    // spectral illuminance/irradiance -> E = rgb * I
    // since the light is directional, we don't need to consider attenuation.
    return clamp( kelvin_to_rgb( light.kelvin ), 0.f, 1.f ) * I;
}


// DIRECTIONAL LIGHT SHADOW TERM
float calculate_shadow_term( in const Light light, in const texture2D shadow_map, in const vec3 world_pos )
{
    // get light space position and perspective divide
    vec4 light_space_position = light.proj * light.view * vec4( world_pos, 1.f );
    light_space_position /= light_space_position.w;

    // get uv coordinates in shadow map and flip y-axis
    vec3 shadow_map_uv = vec3( light_space_position.xy * 0.5f + 0.5f, light_space_position.z );
    // shadow_map_uv.y = 1.f - shadow_map_uv.y;

    return texture( sampler2DShadow( shadow_map, shadow_sampler ), shadow_map_uv );
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
        vec3 E; vec3 L; float shadow_term;
        switch ( light_buffer.lights[i].type )
        {
            case 0: // point light
                E = calculate_point_light_irradiance( light_buffer.lights[i], world_pos.xyz );
                L = normalize( light_buffer.lights[i].position.xyz - world_pos );
                shadow_term = 1.f;
                break;

            case 1: // directional light
                E = calculate_directional_light_irradiance( light_buffer.lights[i], world_pos.xyz );
                L = normalize( light_buffer.lights[i].position.xyz );
                L.y *= -1.f;
                shadow_term = calculate_shadow_term( light_buffer.lights[i], shadow_map_texures[i], world_pos.xyz );
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
        Lo += ( diffuse + specular ) * E * cos_law * shadow_term;
    }

    // calculate global illumination
    const vec3 ambient = calculate_ambient_light( N, V, albedo, metallic, roughness, F0 );

    vec3 final_color = ( Lo + ambient ) * ao * EXPOSURE_COMPENSATION;
    final_color = pow( final_color / ( final_color + vec3( 1.f ) ), vec3( 1.f / 2.2f ) );

    out_color = vec4( final_color.rgb, 1.f );
}