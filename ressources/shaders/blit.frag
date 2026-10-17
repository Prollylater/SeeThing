#version 450 core
in vec2 uv;
layout(binding = 0) uniform sampler2D src;
out vec4 fragment_color;

void main()
{
    fragment_color = texture(src, uv);
}
