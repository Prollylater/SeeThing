#version 450 core
uniform vec3 color;
// Pixels whose mask value is under one half are left alone
uniform bool use_clip;
layout(binding = 1) uniform sampler2D clip;
out vec4 fragment_color;

void main()
{
    if (use_clip && texelFetch(clip, ivec2(gl_FragCoord.xy), 0).r < 0.5)
        discard;
    fragment_color = vec4(color, 1.0);
}
