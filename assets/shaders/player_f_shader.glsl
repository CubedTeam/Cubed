#version 460

in vec2 tc;
in vec3 normal;
in vec3 vert_pos;
in vec4 FragPosLightSpace;
out vec4 color;

layout (binding = 0) uniform sampler2D shadowMap;
layout (binding = 1) uniform sampler2D samp;

uniform float ambientStrength;
uniform vec3 sunlightColor;
uniform vec3 ambientColor;
uniform vec3 sunlightDir;
uniform bool shader_on;
uniform int shadowMode;
uniform float lightSizeUV; 
uniform float minRadius;
uniform float maxRadius;

#include "shadow.glsl"

void main() {
    vec4 objectColor = texture(samp, tc);

    if (!shader_on) {
        color = objectColor;
        return;
    }
   
    vec3 lightDir = normalize(-sunlightDir);
    
    vec3 norm = normalize(normal);

    vec3 ambient = ambientStrength * ambientColor;
    
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 diffuse = diff * sunlightColor;
 
    float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);  
    //float shadow = 0.0;
    //color = vec4(vec3(shadow),1);
    //color = vec4(vec3(diff),1);
    color = vec4((ambient + (1.0 - shadow) * (diffuse)) * objectColor.rgb, objectColor.a);
}
