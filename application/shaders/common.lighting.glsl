#include "common.math.glsl"


// STRUCTS
struct Light
{
    // position for point lights, direction for directional lights
    vec4 position;

    float kelvin;
    float lumen;
    float range;

    // 0: point, 1: directional
    uint type;

    mat4 view;
    mat4 proj;
};


// GEOMETRY
float distribution_ggx( in vec3 N, in vec3 H, in float roughness )
{
    const float a = roughness * roughness;
    const float proj = max( dot( N, H ), 0.f );
    const float proj_squared = proj * proj;

    float denom = proj_squared * ( a - 1.f ) + 1.f;
    denom = PI * denom * denom;

    return a / denom;
}


float geometry_schlick_ggx_direct( in float cos_theta_half, in float roughness )
{
    const float r = roughness + 1.f;
    const float k = ( r * r ) / 8.f;
    return cos_theta_half / ( cos_theta_half * ( 1.f - k ) + k );
}


float geometry_schlick_ggx_indirect( in float cos_theta_half, in float roughness )
{
    const float a = roughness;
    const float k = ( a * a ) / 2.f;
    return cos_theta_half / ( cos_theta_half * ( 1.f - k ) + k );
}


float geometry_smith( in vec3 N, in vec3 V, in vec3 L, in float roughness, in bool indirect )
{
    const float cos_theta_V = max( dot( N, V ), 0.f );
    const float cos_theta_L = max( dot( N, L ), 0.f );
    if ( indirect )
    {
        return geometry_schlick_ggx_indirect( cos_theta_V, roughness ) * geometry_schlick_ggx_indirect( cos_theta_L, roughness );
    }
    return geometry_schlick_ggx_direct( cos_theta_V, roughness ) * geometry_schlick_ggx_direct( cos_theta_L, roughness );
}


vec3 fresnel_schlick( in float cos_theta, in vec3 F0 )
{
    return F0 + ( 1.f - F0 ) * pow( clamp( 1.f - cos_theta, 0.f, 1.f ), 5.f );
}


vec3 fresnel_schlick_roughness( in float cos_theta, in vec3 F0, in float roughness )
{
    return F0 + ( max( vec3( 1.f - roughness ), F0 ) - F0 ) * pow( clamp( 1.f - cos_theta, 0.f, 1.f ), 5.f );
}


// RADIANCE
float radiance_to_luminance( in vec3 Lo )
{
    // human eye’s photopic response curve mapping
    return dot( Lo, vec3( 0.2126f, 0.7152f, 0.0722f ) );
}


// COLOR
vec3 kelvin_to_rgb( in float kelvin )
{
    const float temp = kelvin / 100.f;
    const float r = ( temp <= 66.f ) ? 1.f : clamp( 1.29293618606f * pow( temp - 60.f, -0.1332047592f ), 0.f, 1.f );
    const float g = ( temp <= 66.f ) ?
    clamp( 0.390081578769f * log( temp ) - 0.63184144378f, 0.f, 1.f ) : clamp( 1.12989086089f * pow( temp - 60.f, -0.0755148492f ), 0.f, 1.f );
    const float b = ( temp >= 66.f ) ? 1.f : temp <= 19.f ? 0.f : clamp( 0.54320678911f * log( temp - 10.f ) - 1.19625408914f, 0.f, 1.f );
    return vec3( r, g, b );
}
