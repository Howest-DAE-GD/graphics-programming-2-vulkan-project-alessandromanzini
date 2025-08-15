// STRUCTS
struct Light {
    vec3 position;
    vec3 color;
    float lumen;
    float range;
};


// CONSTANTS
const float PI = 3.14159265358979323846f;


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

float geometry_schlick_ggx( in float cos_theta_half, in float roughness )
{
    const float r = roughness + 1.0;
    const float k = r * r / 8.0;
    return cos_theta_half / ( cos_theta_half * ( 1.0 - k ) + k );
}

float geometry_smith( in vec3 N, in vec3 V, in vec3 L, in float roughness )
{
    const float cos_theta_V = max( dot( N, V ), 0.f );
    const float cos_theta_L = max( dot( N, L ), 0.f );
    return geometry_schlick_ggx( cos_theta_V, roughness ) * geometry_schlick_ggx( cos_theta_L, roughness );
}

vec3 fresnel_schlick( in float cos_theta, in vec3 F0 )
{
    return F0 + ( 1.f - F0 ) * pow( clamp( 1.f - cos_theta, 0.f, 1.f ), 5.f );
}


// RADIANCE
float radiance_to_luminance( in vec3 Lo )
{
    // human eye’s photopic response curve mapping
    return dot( Lo, vec3( 0.2126f, 0.7152f, 0.0722f ) );
}
