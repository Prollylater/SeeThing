#version 450 core
// One quad per dab, a pixel wider than the disc for the soft edge. Positions in pixels, (0, 0) the top-left corner.
layout(location = 0) in vec2 center;
uniform vec2 target_size;
uniform float radius;
out vec2 local; // pixels from the centre

void main()
{
    vec2 corner = vec2(gl_VertexID & 1, gl_VertexID >> 1) * 2.0 - 1.0;
    local = corner * (radius + 1.0);
    gl_Position = vec4((center + local) / target_size * 2.0 - 1.0, 0.0, 1.0);
}
