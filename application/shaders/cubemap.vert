#version 450


// OUTPUT
layout ( location = 0 ) out vec3 out_local_position;


// INPUT
layout ( push_constant ) uniform Push {
    mat4 proj;
    mat4 view;
} pc;


const vec3 VERTEX_POSITIONS[36] = vec3[](
// +X
vec3( 1.f, -1.f, -1.f ), vec3( 1.f, -1.f, 1.f ), vec3( 1.f, 1.f, 1.f ),
vec3( 1.f, -1.f, -1.f ), vec3( 1.f, 1.f, 1.f ), vec3( 1.f, 1.f, -1.f ),

// -X
vec3( -1.f, -1.f, 1.f ), vec3( -1.f, -1.f, -1.f ), vec3( -1.f, 1.f, -1.f ),
vec3( -1.f, -1.f, 1.f ), vec3( -1.f, 1.f, -1.f ), vec3( -1.f, 1.f, 1.f ),

// +Y
vec3( -1.f, 1.f, -1.f ), vec3( 1.f, 1.f, -1.f ), vec3( 1.f, 1.f, 1.f ),
vec3( -1.f, 1.f, -1.f ), vec3( 1.f, 1.f, 1.f ), vec3( -1.f, 1.f, 1.f ),

// -Y
vec3( -1.f, -1.f, 1.f ), vec3( 1.f, -1.f, 1.f ), vec3( 1.f, -1.f, -1.f ),
vec3( -1.f, -1.f, 1.f ), vec3( 1.f, -1.f, -1.f ), vec3( -1.f, -1.f, -1.f ),

// +Z
vec3( 1.f, -1.f, 1.f ), vec3( -1.f, -1.f, 1.f ), vec3( -1.f, 1.f, 1.f ),
vec3( 1.f, -1.f, 1.f ), vec3( -1.f, 1.f, 1.f ), vec3( 1.f, 1.f, 1.f ),

// -Z
vec3( -1.f, -1.f, -1.f ), vec3( 1.f, -1.f, -1.f ), vec3( 1.f, 1.f, -1.f ),
vec3( -1.f, -1.f, -1.f ), vec3( 1.f, 1.f, -1.f ), vec3( -1.f, 1.f, -1.f )
);


void main( )
{
    const vec3 position = VERTEX_POSITIONS[gl_VertexIndex].rgb;
    out_local_position = position;
    gl_Position = pc.proj * pc.view * vec4( position, 1.f );
}