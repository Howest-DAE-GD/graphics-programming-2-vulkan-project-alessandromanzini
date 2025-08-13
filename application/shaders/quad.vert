#version 450

// OUTPUT
layout ( location = 0 ) out vec2 out_uv;


// SHADER ENTRY POINT
void main( )
{
    out_uv = vec2( ( gl_VertexIndex << 1 ) & 2, gl_VertexIndex & 2 );
    gl_Position = vec4( out_uv * 2.f - 1.f, 0.0f, 1.0f );
}