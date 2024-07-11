#version 330 core
in vec2 vertex_texcoord;
uniform sampler2D image_color;

void main()
{
    color= texture(image_color, vertex_texcoord);
}
