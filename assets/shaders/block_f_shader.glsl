#version 460

in vec2 tc;
in vec3 normal;
in vec3 vert_pos;
in vec3 tangent;
in vec3 bitangent;
in vec4 FragPosLightSpace;
in float roughness;
flat in int tex_layer;
out vec4 color;
layout (binding = 0) uniform sampler2D shadowMap;
layout (binding = 1) uniform sampler2DArray samp;
layout (binding = 2) uniform sampler2DArray normMap;
uniform float ambientStrength;
uniform vec3 sunlightColor;
uniform vec3 ambientColor;
uniform vec3 sunlightDir;
uniform vec3 cameraPos;
uniform bool shader_on;
uniform int shadowMode;
uniform float specularStrength;
uniform float lightSizeUV; 
uniform float minRadius;
uniform float maxRadius;
uniform bool enablePBR;
uniform bool flipY;

uniform int renderDistance;
uniform vec3 skyColor;

#include "shadow.glsl"
#include "normal.glsl"

void main(void) {
    vec4 objectColor = texture(samp, vec3(tc, tex_layer));

    if (objectColor.a < 0.8) {
        discard;
    }
    if (!shader_on) {
        color = objectColor;
        return;
    }
   

    vec3 lightDir = normalize(-sunlightDir);
    vec3 norm; 
    if (enablePBR) {
        norm = calcNewNormal();
    } else {
        norm = normalize(normal);
    }


    vec3 V =
        normalize(cameraPos - vert_pos);

    vec3 H = 
        normalize(lightDir + V);

    vec3 ambient = ambientStrength * ambientColor;
    
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 diffuse = diff * sunlightColor;
    
    float r =
        clamp(roughness, 0.0, 1.0);

    float shininess =
        mix(
            512.0,
            4.0,
            r
        );
    float ks =
        mix(
            0.8,
            0.02,
            r
        );

    float spec = 0.0;

    if(diff > 0.0)
    {
        spec =
            ks *
            pow(
                max(dot(norm,H),0.0),
                shininess
            );
    }



    
    vec3 specular = spec * sunlightColor * specularStrength;

    float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);  
    
    // fog
    float dist = length(cameraPos - vert_pos);
    vec4 fogColor = vec4(skyColor, 1.0);
    float fogStart = renderDistance * 16 * 0.9;
    float fogEnd   = renderDistance * 16;

    float fogFactor = smoothstep(fogEnd, fogStart, dist);
    color = vec4((ambient + (1.0 - shadow) * (diffuse)) * objectColor.rgb + (1.0-shadow) * specular * objectColor.rgb, objectColor.a);
    
    color = mix(fogColor, color, fogFactor);
    //color = vec4(normal * 0.5 + 0.5, 1.0);
    //color = vec4(tangent * 0.5 + 0.5, 1.0);;
    //color = vec4(norm * 0.5 + 0.5, 1.0);
    //color = vec4(calcNewNormal(), 1.0);
}
