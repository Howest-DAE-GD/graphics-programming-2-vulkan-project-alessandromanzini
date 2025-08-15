// ENCODING
// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
vec2 octa_wrap( in vec2 v )
{
    return ( 1.f - abs( v.yx ) ) * vec2( v.x >= 0.f ? 1.f : -1.f, v.y >= 0.f ? 1.f : -1.f );
}

// https://stackoverflow.com/questions/35486775/how-do-i-convert-between-float-and-vec4-vec3-vec2
vec2 float_to_vec2( in float v, float min, float max )
{
    const float zero_to_one = ( v - min ) / ( max - min );
    const float zero_to_16bit = zero_to_one * 256.f * 255.f;
    return vec2( mod( zero_to_16bit, 256.f ), zero_to_16bit / 256.f );
}

vec2 encode16( in vec3 N )
{
    // octahedron normal encoding
    N /= ( abs( N.x ) + abs( N.y ) + abs( N.z ) );
    N.xy = ( N.z >= 0.f ) ? N.xy : octa_wrap( N.xy );
    N.xy = N.xy * .5f + vec2( .5f, .5f );
    return N.xy;
}

vec4 encode32( in vec3 N )
{
    vec2 N16 = encode16( N );
    return vec4( float_to_vec2( N16.x, 0.f, 1.f ), float_to_vec2( N16.y, 0.f, 1.f ) );
}


// DECODING
// https://stackoverflow.com/questions/35486775/how-do-i-convert-between-float-and-vec4-vec3-vec2
float vec2_to_float( in vec2 v, float min, float max )
{
    const float zero_to_16bit = v.x + v.y * 256.f;
    const float zero_to_one = zero_to_16bit / 256.f / 255.f;
    return zero_to_one * ( max - min ) + min;
}

// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
vec3 decode16( in vec2 f16 )
{
    f16 = f16 * 2.f - vec2( 1.f, 1.f );

    // https://twitter.com/Stubbesaurus/status/937994790553227264
    vec3 N = vec3( f16.x, f16.y, 1.f - abs( f16.x ) - abs( f16.y ) );
    const float t = clamp( -N.z, 0.f, 1.f );
    N.xy += vec2( ( N.x >= 0.f ) ? -t : t, ( N.y >= 0.f ) ? -t : t );
    return normalize( N );
}

vec3 decode32( in vec4 f32 )
{
    vec2 f16 = vec2( vec2_to_float( f32.xy, 0.f, 1.f ), vec2_to_float( f32.zw, 0.f, 1.f ) );
    return decode16( f16 );
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
