#version 460

in vec2 tc;
in vec3 normal;
in vec3 vert_pos;

out vec4 color;

layout (binding = 0) uniform sampler2D samp;


void main() {
    color = texture(samp, tc);

}
