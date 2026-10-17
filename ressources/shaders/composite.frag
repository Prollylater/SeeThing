#version 450 core
// Source over backdrop with a separable blend mode, W3C Compositing and Blending Level 1, on sRGB values.
// Both inputs and the output are straight alpha; the formula works premultiplied.
layout(binding = 0) uniform sampler2D backdrop;
layout(binding = 1) uniform sampler2D source;
uniform float opacity;
uniform int mode; // core::BlendMode
out vec4 fragment_color;

vec3 blendColor(vec3 cb, vec3 cs)
{
    switch (mode)
    {
    case 1: return cb * cs;
    case 2: return cb + cs - cb * cs;
    // HardLight with the roles swapped
    case 3: return mix(2.0 * cb * cs, 1.0 - 2.0 * (1.0 - cb) * (1.0 - cs), step(0.5, cb));
    case 4: return abs(cb - cs);
    }
    return cs;
}

void main()
{
    ivec2 p = ivec2(gl_FragCoord.xy);
    vec4 b = texelFetch(backdrop, p, 0);
    vec4 s = texelFetch(source, p, 0);
    float as = s.a * opacity;
    float ab = b.a;
    // Where the backdrop is transparent the source shows unblended
    vec3 mixed = (1.0 - ab) * s.rgb + ab * blendColor(b.rgb, s.rgb);
    float ao = as + ab * (1.0 - as);
    vec3 co = as * mixed + ab * (1.0 - as) * b.rgb;
    fragment_color = ao > 0.0 ? vec4(co / ao, ao) : vec4(0.0);
}
