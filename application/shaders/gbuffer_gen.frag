#version 450
#extension GL_EXT_nonuniform_qualifier: enable

// STRUCTS
struct SurfaceMap
{
    int diffuse_id;
    int normal_id;
    int metalness_id;
    int roughness_id;
};


// INPUT
layout ( location = 0 ) in vec2 frag_uv;
layout ( location = 1 ) in mat3 frag_tbn;


// OUTPUT
layout ( location = 0 ) out vec4 out_albedo;
layout ( location = 1 ) out vec4 out_normal;
// layout ( location = 2 ) out vec4 out_material;


// BINDINGS
layout ( push_constant ) uniform PushConstants {
    uint surface_id;
} pc;
layout ( constant_id = 0 ) const uint TEXTURE_COUNT = 1u;
layout ( set = 0, binding = 1 ) uniform sampler shared_sampler;
layout ( set = 0, binding = 2 ) uniform texture2D textures[TEXTURE_COUNT];
layout ( set = 0, binding = 3 ) readonly buffer SurfaceData
{
    SurfaceMap maps[];
} surface_buffer;


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

vec2 encode( vec3 n )
{
    // Normal world space to tangent space
    n = normalize( frag_tbn * ( n * 2.0 - vec3( 1.0, 1.0, 1.0 ) ) );

    // Octahedron normal encoding
    n /= ( abs( n.x ) + abs( n.y ) + abs( n.z ) );
    n.xy = ( n.z >= 0.f ) ? n.xy : octa_wrap( n.xy );
    n.xy = n.xy * .5f + vec2( .5f, .5f );
    return n.xy;
}


// SHADER ENTRY POINT
void main( )
{
    SurfaceMap map = surface_buffer.maps[pc.surface_id];
    const int diffuse_id = nonuniformEXT( map.diffuse_id );
    const int normal_id = nonuniformEXT( map.normal_id );
    // const int metalness_id = nonuniformEXT( map.metalness_id );
    // const int roughness_id = nonuniformEXT( map.roughness_id );

    vec3 normal = texture( sampler2D( textures[normal_id], shared_sampler ), frag_uv ).rgb;
    // const float metalness = texture( sampler2D( textures[metalness_id], shared_sampler ), frag_uv ).b;
    // const float roughness = texture( sampler2D( textures[roughness_id], shared_sampler ), frag_uv ).g;

    out_albedo = texture( sampler2D( textures[diffuse_id], shared_sampler ), frag_uv ).rgba;
    out_normal = vec4( encode( normal ), 0.f, 0.f );
}