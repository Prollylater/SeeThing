#version 330 core
layout(location= 0) in vec3 position;
//layout(location= 1) in vec2 texcoord;

uniform int text_width;
uniform int text_height;
//uniform int pt_size;

void main()
{
    gl_PointSize = 2; 
    // Convert coordinate to ndc range [-1, 1]
    vec3 normalized_coord = vec3(position.x / text_width , position.y /text_height , position.z);
    normalized_coord = 2*(normalized_coord) - 1.0; 
    
    gl_Position = vec4(normalized_coord.xyz, 1.0);


}
