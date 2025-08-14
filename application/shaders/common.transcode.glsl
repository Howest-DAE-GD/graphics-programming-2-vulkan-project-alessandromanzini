
// ENCODING
// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
vec2 octa_wrap( vec2 v )
{
    return ( 1.0 - abs( v.yx ) ) * vec2( v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0 );
}

// https://stackoverflow.com/questions/35486775/how-do-i-convert-between-float-and-vec4-vec3-vec2
vec2 float_to_vec2( float zero_to_one )
{
    float zero_to_16bit = zero_to_one * 256.f * 255.f;
    return vec2( mod( zero_to_16bit, 256.f ), zero_to_16bit / 256.f );
}

vec2 encode( vec3 N, mat3 TBN )
{
    // Normal world space to tangent space
    N = normalize( TBN * ( N * 2.0 - vec3( 1.0, 1.0, 1.0 ) ) );

    // Octahedron normal encoding
    N /= ( abs( N.x ) + abs( N.y ) + abs( N.z ) );
    N.xy = ( N.z >= 0.f ) ? N.xy : octa_wrap( N.xy );
    N.xy = N.xy * .5f + vec2( .5f, .5f );
    return N.xy;
}


// DECODING
// https://stackoverflow.com/questions/35486775/how-do-i-convert-between-float-and-vec4-vec3-vec2
float vec2_to_float( vec2 v, float min, float max )
{
    const float zero_to_16bit = v.x + v.y * 256.f;
    const float zero_to_one = zero_to_16bit / 256.f / 255.f;
    return zero_to_one * ( max - min ) + min;
}

// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
vec3 decode( vec2 f )
{
    f = normalize( f ) * 2.f - vec2( 1.f, 1.f );

    // https://twitter.com/Stubbesaurus/status/937994790553227264
    vec3 n = vec3( f.x, f.y, 1.f - abs( f.x ) - abs( f.y ) );
    const float t = clamp( -n.z, 0.f, 1.f );
    n.xy += vec2( ( n.x >= 0.f ) ? -t : t, ( n.y >= 0.f ) ? -t : t );
    return normalize( n );
}


// WORLD POSITION
// https://stackoverflow.com/questions/32227283/getting-world-position-from-depth-buffer-value
vec3 get_world_pos_from_depth( float depth, vec2 tex_coord, mat4 proj, mat4 view )
{
    vec4 clip_space_pos = vec4( tex_coord * 2.0 - vec2( 1.0, 1.0 ), depth, 1.0 );
    vec4 view_space_pos = inverse( proj ) * clip_space_pos;

    // perspective divide
    view_space_pos /= view_space_pos.w;

    vec4 world_space_pos = inverse( view ) * view_space_pos;
    return world_space_pos.xyz;
}
