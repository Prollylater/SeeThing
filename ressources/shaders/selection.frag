#version 450 core
// Drawn inside ImGui's pass, over the canvas: dims what is outside the selection and runs marching ants
// along its inner edge. Works in framebuffer pixels, so the ants keep their size at any zoom.
layout(binding = 1) uniform sampler2D mask;
uniform vec2 viewport_pos;   
uniform float fb_scale;      // framebuffer pixels per ImGui unit
uniform float fb_height;
uniform vec2 image_origin;  
uniform float zoom;
uniform float time;
out vec4 fragment_color;

ivec2 texelAt(vec2 frag)
{
    vec2 screen = viewport_pos + frag / fb_scale;
    return ivec2(floor((screen - image_origin) / zoom));
}

bool inImage(ivec2 t)
{
    return all(greaterThanEqual(t, ivec2(0))) && all(lessThan(t, textureSize(mask, 0)));
}

bool selected(vec2 frag)
{
    ivec2 t = texelAt(frag);
    return inImage(t) && texelFetch(mask, t, 0).r >= 0.5;
}

void main()
{
    vec2 frag = vec2(gl_FragCoord.x, fb_height - gl_FragCoord.y);
    if (selected(frag))
    {
        bool edge = !selected(frag + vec2(1, 0)) || !selected(frag - vec2(1, 0)) ||
                    !selected(frag + vec2(0, 1)) || !selected(frag - vec2(0, 1));
        if (!edge)
            discard;
        float stripe = mod(floor((frag.x + frag.y) / 4.0 - time * 8.0), 2.0);
        fragment_color = vec4(vec3(stripe), 1.0);
    }
    else if (inImage(texelAt(frag)))
        fragment_color = vec4(0.0, 0.0, 0.0, 0.45);
    else
        discard;
}
