#version 450
#extension GL_EXT_nonuniform_qualifier: enable

#include "common.surface.glsl"


// INPUT
layout ( location = 0 ) in vec2 in_uv;


// BINDINGS
layout ( push_constant ) uniform SurfaceDescription {
    uint surface_id;
} SD;

layout ( set = 0, binding = 1 ) readonly buffer SurfaceBufferData { SurfaceMap maps[]; } surface_buffer;

layout ( constant_id = 0 ) const uint TEXTURE_COUNT = 1u;
layout ( set = 1, binding = 0 ) uniform sampler shared_sampler;
layout ( set = 1, binding = 1 ) uniform texture2D textures[TEXTURE_COUNT];


// SHADER ENTRY POINT
void main( )
{
    SurfaceMap map = surface_buffer.maps[SD.surface_id];

    float alpha = texture( sampler2D( textures[nonuniformEXT( map.base_color_id )], shared_sampler ), in_uv ).a;
    if ( alpha < 0.95f )
    {
        discard;
    }
}