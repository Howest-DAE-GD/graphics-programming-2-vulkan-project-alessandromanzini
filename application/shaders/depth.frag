#version 450
#extension GL_EXT_nonuniform_qualifier: enable

#include "common.surface.glsl"
#include "common.texture.glsl"


// INPUT
layout ( location = 0 ) in vec2 frag_uv;


// SHADER ENTRY POINT
void main( )
{
    SurfaceMap map = surface_buffer.maps[SD.surface_id];

    float alpha = texture( sampler2D( textures[nonuniformEXT( map.base_color_id )], shared_sampler ), frag_uv ).a;
    if ( alpha < 0.95f )
    {
        discard;
    }
}