#version 460

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform mat4 lightSpaceMatrix;
uniform mat4 modelMatrix;
out vec4 FragPosLightSpace;
out vec3 normal;
out vec2 tc;
out vec3 vert_pos;


void main() {
    vec4 worldPos = modelMatrix * vec4(pos, 1.0);
    FragPosLightSpace = lightSpaceMatrix * worldPos;
    vec4 viewPos = mv_matrix * vec4(pos, 1.0);
    tc = texCoord;
    vert_pos = pos;
    normal = normalize(mat3(norm_matrix) * aNormal);
    gl_Position = proj_matrix * viewPos;
}