#version 450 core
// A stroke's coverage applied to its layer. Paint: the colour over the layer. Erase: W3C destination-out,
// which keeps the colour and removes alpha. Opacity caps the whole stroke, so overlapping dabs never pass it.
layout(binding = 0) uniform sampler2D layer;
layout(binding = 1) uniform sampler2D coverage;
layout(binding = 2) uniform sampler2D clip;
uniform vec3 color;
uniform float opacity;
uniform bool erase;
uniform bool use_clip;
out vec4 fragment_color;

void main()
{
    ivec2 p = ivec2(gl_FragCoord.xy);
    vec4 l = texelFetch(layer, p, 0);
    float a = texelFetch(coverage, p, 0).r * opacity;
    if (use_clip)
        a *= texelFetch(clip, p, 0).r;
    if (erase)
    {
        fragment_color = vec4(l.rgb, l.a * (1.0 - a));
        return;
    }
    float ao = a + l.a * (1.0 - a);
    vec3 co = ao > 0.0 ? (color * a + l.rgb * l.a * (1.0 - a)) / ao : vec3(0.0);
    fragment_color = vec4(co, ao);
}
