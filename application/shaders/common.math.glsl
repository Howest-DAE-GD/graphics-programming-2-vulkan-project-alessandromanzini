// CONSTANTS
const vec2 INV_TAN = vec2( 0.1591f, 0.3183f ); // 1/(2π), 1/π
const float PI = 3.14159265358979323846f;


// SAMPLE SPHERICAL MAP
vec2 sample_spherical_map( in vec3 V )
{
    vec2 uv = vec2( atan( V.z, V.x ), asin( V.y ) );
    uv *= INV_TAN;
    uv += 0.5f;
    return uv;
}


// TANGENT TO WORLD
void tangent_to_world( in vec3 N, out vec3 tangent, out vec3 bitangent )
{
    vec3 up = abs( N.z ) < 0.999 ? vec3( 0.0, 0.0, 1.0 ) : vec3( 0.0, 1.0, 0.0 );

    tangent = normalize( cross( up, N ) );
    bitangent = cross( N, tangent );
}


// WORLD POSITION
// https://stackoverflow.com/questions/32227283/getting-world-position-from-depth-buffer-value
vec3 get_world_pos_from_depth( in float depth, in vec2 tex_coord, in mat4 proj, in mat4 view )
{
    vec4 clip_space_pos = vec4( tex_coord * 2.f - vec2( 1.f, 1.f ), depth, 1.f );
    vec4 view_space_pos = inverse( proj ) * clip_space_pos;

    // perspective divide
    view_space_pos /= view_space_pos.w;

    vec4 world_space_pos = inverse( view ) * view_space_pos;
    return world_space_pos.xyz;
}