#version 450

// INPUT
layout ( location = 0 ) in vec2 frag_uv;


// OUTPUT
layout ( location = 0 ) out vec4 out_color;


// BINDINGS
layout ( push_constant ) uniform PushConstants {
    vec3 view;
} pc;
layout ( set = 0, binding = 1 ) uniform sampler shared_sampler;
layout ( set = 0, binding = 4 ) uniform texture2D albedo_texture;
layout ( set = 0, binding = 5 ) uniform texture2D normal_texture;


// DECODING
// https://stackoverflow.com/questions/35486775/how-do-i-convert-between-float-and-vec4-vec3-vec2
float vec2_to_float( vec2 v, float min, float max )
{
    float zero_to_16bit = v.x + v.y * 256.f;
    float zero_to_one = zero_to_16bit / 256.f / 255.f;
    return zero_to_one * ( max - min ) + min;
}

// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
vec3 decode( vec2 f )
{
    f = normalize( f ) * 2.f - vec2( 1.f, 1.f );

    // https://twitter.com/Stubbesaurus/status/937994790553227264
    vec3 n = vec3( f.x, f.y, 1.f - abs( f.x ) - abs( f.y ) );
    float t = clamp( -n.z, 0.f, 1.f );
    n.xy += vec2( ( n.x >= 0.f ) ? -t : t, ( n.y >= 0.f ) ? -t : t );
    return normalize( n );
}


// SHADER ENTRY POINT
void main( )
{
    vec4 albedo = texture( sampler2D( albedo_texture, shared_sampler ), frag_uv ).rgba;
    vec3 normal = decode( texture( sampler2D( normal_texture, shared_sampler ), frag_uv ).rg );

    vec3 light_dir = normalize( vec3( 0.f, -1.f, 0.f ) );
    float cos_law = max( dot( normal, -light_dir ), 0.f );

    out_color = vec4( albedo.rgb * cos_law, albedo.a );
}