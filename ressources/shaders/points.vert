#version 450 core
// One size x size square per instance, centered on a pixel. (0, 0) is the top-left pixel, row 0 of the target.
layout(location = 0) in vec2 pixel;
uniform vec2 target_size;
uniform float point_size;

void main()
{
    // Strip order (0, 0), (1, 0), (0, 1), (1, 1)
    vec2 corner = vec2(gl_VertexID & 1, gl_VertexID >> 1);
    vec2 p = pixel + 0.5 + (corner - 0.5) * point_size;
    gl_Position = vec4(p / target_size * 2.0 - 1.0, 0.0, 1.0);
}
