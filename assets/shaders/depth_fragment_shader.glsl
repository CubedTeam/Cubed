#version 460

in vec2 tc;
flat in int tex_layer;
layout (binding = 1) uniform sampler2DArray samp;

uniform bool is_discard_tranparent;

void main() {
    if (is_discard_tranparent) {
        vec4 texColor = texture(samp, vec3(tc, tex_layer));
        if (texColor.a < 0.8) 
            discard;
    }   
    //gl_FragDepth = gl_FragCoord.z; 
}