#version 450

// INPUT
layout ( location = 0 ) in vec2 frag_uv;


// OUTPUT
layout ( location = 0 ) out vec4 out_color;


// BINDINGS
layout ( set = 0, binding = 1 ) uniform sampler shared_sampler;
layout ( set = 0, binding = 4 ) uniform texture2D albedo_texture;


// SHADER ENTRY POINT
void main( )
{
    vec4 albedo = texture( sampler2D( albedo_texture, shared_sampler ), frag_uv ).rgba;

    out_color = albedo;
}