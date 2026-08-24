#version 460

uniform sampler2DArray breakTextures;
uniform int breakStage;

in vec2 uv;
out vec4 fragColor;

void main()
{
    vec4 color = texture(breakTextures, vec3(uv, float(breakStage)));

    if (color.a < 0.01) {
        discard;
    }

    fragColor = color;
}