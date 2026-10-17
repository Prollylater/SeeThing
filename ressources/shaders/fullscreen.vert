#version 450 core
// One triangle covering the target, no vertex buffer. uv (0, 0) is row 0 of the source.
out vec2 uv;

void main()
{
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    uv = p;
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
