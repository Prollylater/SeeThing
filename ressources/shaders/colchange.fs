#version 330 core
out vec4 fragment_color;
uniform vec3 new_color;    


void main() {
        fragment_color = vec4(new_color, 1.0);
}

