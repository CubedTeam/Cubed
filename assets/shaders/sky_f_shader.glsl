#version 460

in vec3 dir;

out vec4 frag_color;


uniform vec3 skyTop;
uniform vec3 skyBottom;

uniform vec3 sunDir;
uniform vec3 sunColor;

uniform float horizonSharpness;
uniform float cloudWhiteMix;

uniform float cloudThresholdLow; 
uniform float cloudThresholdHigh;

uniform float time;

#include "compute_sky_color.glsl"

void main(void) {
    
    vec3 sky = computeSkyColor(dir);

    frag_color = vec4(sky, 1.0);
    
}