#version 450


// OUTPUT
layout ( location = 0 ) out vec4 out_color;


// BINDING
layout ( set = 0, binding = 0 ) uniform sampler shared_sampler;
layout ( set = 0, binding = 1 ) uniform texture2D color_texture;


// SHADER ENTRY POINT
void main( )
{
    const ivec2 ifrag_coord = ivec2( gl_FragCoord.xy );
    out_color = vec4( texelFetch( sampler2D( color_texture, shared_sampler ), ifrag_coord, 0 ).rbg, 1.f );
}
