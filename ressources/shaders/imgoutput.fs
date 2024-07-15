#version 330 core
in vec2 UV;
uniform sampler2D image_color;
out vec4 fragment_color;

void main()
{
    fragment_color= texture(image_color, UV);
   // fragment_color= vec4(0.5,0.3,0.7,1.0);

}
