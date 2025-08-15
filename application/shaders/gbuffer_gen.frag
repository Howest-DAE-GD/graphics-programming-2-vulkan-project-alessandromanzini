#version 450
#extension GL_EXT_nonuniform_qualifier: enable

#include "common.surface.glsl"
#include "common.transcode.glsl"


// INPUT
layout ( location = 0 ) in vec2 in_uv;
layout ( location = 1 ) in mat3 in_TBN;


// OUTPUT
layout ( location = 0 ) out vec4 out_albedo;
layout ( location = 1 ) out vec4 out_material;


// BINDINGS
layout ( constant_id = 0 ) const uint TEXTURE_COUNT = 1u;
layout ( set = 0, binding = 1 ) uniform sampler shared_sampler;
layout ( set = 0, binding = 2 ) uniform texture2D textures[TEXTURE_COUNT];


// SHADER ENTRY POINT
void main( )
{
    SurfaceMap map = surface_buffer.maps[SD.surface_id];

    const vec3 albedo = texture( sampler2D( textures[nonuniformEXT( map.base_color_id )], shared_sampler ), in_uv ).rgb;
    const float metalness = texture( sampler2D( textures[nonuniformEXT( map.metalness_id )], shared_sampler ), in_uv ).b;
    const float roughness = texture( sampler2D( textures[nonuniformEXT( map.roughness_id )], shared_sampler ), in_uv ).g;
    const float ao = texture( sampler2D( textures[nonuniformEXT( map.ao_id )], shared_sampler ), in_uv ).r;

    vec3 normal = texture( sampler2D( textures[nonuniformEXT( map.normal_id )], shared_sampler ), in_uv ).rgb;
    normal = normalize( in_TBN * ( normal * 2.f - vec3( 1.f, 1.f, 1.f ) ) );

    out_albedo = vec4( albedo, ao );
    out_material = vec4( encode16( normal ).rg, metalness, roughness );
}