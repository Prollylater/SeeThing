#version 450 core
// A round dab into the stroke's coverage: 1 inside, falling to 0 at the rim. Hardness is where the fall starts;
// the edge always spans at least the pixel across the rim, so a hard dab is antialiased and keeps its true area.
in vec2 local;
uniform float radius;
uniform float hardness;
uniform float flow;
out vec4 fragment_color;

void main()
{
    float d = length(local);
    float a = (1.0 - smoothstep(min(hardness * radius, radius - 0.5), radius + 0.5, d)) * flow;
    // Blended ONE, ONE_MINUS_SRC_ALPHA: coverage builds up dab over dab and never passes 1
    fragment_color = vec4(a);
}
