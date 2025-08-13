#version 450
#extension GL_EXT_nonuniform_qualifier: enable

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


// OUTPUT
layout ( location = 0 ) out vec4 out_albedo;


// BINDINGS
layout ( push_constant ) uniform PushConstants {
    uint material_id;
} pc;
layout ( constant_id = 0 ) const uint TEXTURE_COUNT = 1u;
layout ( set = 0, binding = 1 ) uniform sampler shared_sampler;
layout ( set = 0, binding = 2 ) uniform texture2D textures[TEXTURE_COUNT];
layout ( set = 0, binding = 3 ) readonly buffer MaterialData
{
    Material materials[];
} material_buffer;


// SHADER ENTRY POINT
void main( )
{
    Material material = material_buffer.materials[pc.material_id];

    const int diffuse_id = nonuniformEXT( material.diffuse_id );
    out_albedo = texture( sampler2D( textures[diffuse_id], shared_sampler ), frag_uv ).rgba;
}