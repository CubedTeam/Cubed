#version 460

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

out vec2 uv;

void main()
{
    uv = vertexUV;
    gl_Position = proj_matrix * mv_matrix * vec4(vertexPosition, 1.0);
}