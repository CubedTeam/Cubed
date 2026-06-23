#include "noise.glsl"

vec3 computeSkyColor(vec3 dir) {
    vec3 sund = normalize(sunDir);

    float t =
        clamp(
            dir.y * 0.5 + 0.5,
            0.0,
            1.0
        );


    vec3 sky =
        mix(
            skyBottom,
            skyTop,
            pow(t, horizonSharpness)
        );

    // cloud
    if (dir.y > 0.0) {
        vec2 cloud_uv = dir.xz / (dir.y + 0.15) * 0.5 + vec2(time * 0.005, time * 0.002);
        float cloud_density = fbm(cloud_uv * 2.0);
        float safeLow = cloudThresholdLow;
        float safeHigh = max(cloudThresholdHigh, cloudThresholdLow + 0.001);
        cloud_density = smoothstep(safeLow,safeHigh, cloud_density);

        
        float fade = smoothstep(0.0, 0.3, dir.y) * (1.0 - smoothstep(0.85, 1.0, dir.y));
        cloud_density *= fade;

        vec3 cloud_color = mix(skyBottom, vec3(1.0), cloudWhiteMix); 
        sky = mix(sky, cloud_color, cloud_density * 0.6);
    }

    float sunAmount = max(dot(dir, sund), 0.0);
        
    //float glow = pow(sunAmount, 8.0) * 0.15;
        
    float glow = pow(sunAmount, 8.0) * 0.15 + pow(sunAmount, 32.0) * 0.3;

    sky += glow * sunColor;

    return sky;
}