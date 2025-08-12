#version 450
#extension GL_EXT_nonuniform_qualifier : enable

// STRUCTS
struct Material
{
    int diffuse_id;
    int specular_id;
    int normal_id;
    int metalness_id;
};


// INPUT
layout ( location = 0 ) in vec2 frag_uv;


// PUSH CONSTANTS
layout ( push_constant ) uniform PushConstants {
    uint material_id;
} pc;


// OUTPUT
layout ( location = 0 ) out vec4 out_color;


// BINDINGS
layout ( set = 0, binding = 1 ) uniform sampler shared_sampler;
layout ( set = 0, binding = 2 ) uniform texture2D textures[24];
layout ( set = 0, binding = 3 ) readonly buffer MaterialData
{
    Material materials[];
} material_buffer;


// The triangle that is formed by the positions from the vertex shader fills an area on the screen with fragments.
// The fragment shader is invoked on these fragments to produce a color and depth for the frame buffer.
// The main function is called for every fragment and will provide automatic fragment interpolation.
void main( )
{
    Material material = material_buffer.materials[pc.material_id];

    const int diffuse_id = nonuniformEXT( material.diffuse_id );
    vec4 diffuse_color = texture( sampler2D( textures[diffuse_id], shared_sampler ), frag_uv ).rgba;

    // alpha cutout
    if ( diffuse_color.a < 0.95 )
    {
        discard;
    }

    out_color = diffuse_color;
}