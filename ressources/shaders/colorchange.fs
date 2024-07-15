#version 330 core
out vec4 fragment_color;
uniform vec2 pixel_coord; //Pixel to change 
uniform vec3 new_color;    
uniform uint  texture_size;

void main() {
    // Convert coordinate to viewport coordinate range [-1, 1]
    vec2 normalized_coord = pixel_coord / vec2(texture_size);
    normalized_coord = normalized_coord - 1.0; 
    
    //  TODO if alternative
    if(abs(gl_FragCoord.xy - pixel_coord) < vec2(0.5)) {
        fragment_color = vec3(new_color, 1.0);
    } 
}
}

