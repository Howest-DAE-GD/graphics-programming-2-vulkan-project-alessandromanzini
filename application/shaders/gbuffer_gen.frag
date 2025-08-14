#version 450
#extension GL_EXT_nonuniform_qualifier: enable

#include "common.surface.glsl"
#include "common.texture.glsl"
#include "common.transcode.glsl"


// INPUT
layout ( location = 0 ) in vec2 in_uv;
layout ( location = 1 ) in mat3 in_TBN;


// OUTPUT
layout ( location = 0 ) out vec4 out_albedo;
layout ( location = 1 ) out vec4 out_material;


// SHADER ENTRY POINT
void main( )
{
    SurfaceMap map = surface_buffer.maps[SD.surface_id];

    vec3 albedo = texture( sampler2D( textures[nonuniformEXT( map.base_color_id )], shared_sampler ), in_uv ).rgb;
    vec3 normal = texture( sampler2D( textures[nonuniformEXT( map.normal_id )], shared_sampler ), in_uv ).rgb;
    const float metalness = texture( sampler2D( textures[nonuniformEXT( map.metalness_id )], shared_sampler ), in_uv ).b;
    const float roughness = texture( sampler2D( textures[nonuniformEXT( map.roughness_id )], shared_sampler ), in_uv ).g;
    const float ao = texture( sampler2D( textures[nonuniformEXT( map.ao_id )], shared_sampler ), in_uv ).r;

    out_albedo = vec4( albedo, ao );
    out_material = vec4( encode( normal, in_TBN ), metalness, roughness );
}