#version 460

layout (location = 0) in vec2 pos;


uniform mat4 model_matrix;
uniform mat4 proj_matrix;

void main(void) {
    gl_Position = proj_matrix * model_matrix * vec4(pos, 0.0, 1.0);
}
