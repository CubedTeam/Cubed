#version 460

layout (binding = 0) uniform sampler2D samp;

out vec4 color;
in vec2 tc;

void main() {
    color = texture(samp, tc);
}