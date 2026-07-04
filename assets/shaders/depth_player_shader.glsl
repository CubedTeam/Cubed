#version 460

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;

uniform mat4 lightSpaceMatrix;
uniform mat4 modelMatrix;

out vec2 tc;
flat out int tex_layer;

void main() {
    tc = texCoord;
    gl_Position = lightSpaceMatrix * modelMatrix * vec4(pos, 1.0);
}