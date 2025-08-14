
// STRUCTS
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};


// CONSTANTS
const float PI = 3.14159265358979323846f;


// FUNCTIONS
float distribution_ggx( vec3 N, vec3 H, float roughness )
{
    const float a = roughness * roughness;
    const float proj = max( dot( N, H ), 0.f );
    const float proj_squared = proj * proj;

    float denom = proj_squared * ( a - 1.f ) + 1.f;
    denom = PI * denom * denom;

    return a / denom;
}

float geometry_schlick_ggx( float cos_theta_half, float roughness )
{
    const float r = roughness + 1.0;
    const float k = r * r / 8.0;
    return cos_theta_half / ( cos_theta_half * ( 1.0 - k ) + k );
}

float geometry_smith( vec3 N, vec3 V, vec3 L, float roughness )
{
    const float cos_theta_V = max( dot( N, V ), 0.f );
    const float cos_theta_L = max( dot( N, L ), 0.f );
    return geometry_schlick_ggx( cos_theta_V, roughness ) * geometry_schlick_ggx( cos_theta_L, roughness );
}

vec3 fresnel_schlick( float cos_theta, vec3 F0 )
{
    return F0 + ( 1.f - F0 ) * pow( clamp( 1.f - cos_theta, 0.f, 1.f ), 5.f );
}
