#version 460

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec3 outSDR;

layout(set = 0, binding = 0) uniform sampler2D uHDR;
layout(set = 0, binding = 1) uniform sampler2D uBloom;

const mat3 ACESInputMat = mat3(
    vec3(0.59719, 0.07600, 0.02840),
    vec3(0.35458, 0.90834, 0.13383),
    vec3(0.04823, 0.01566, 0.83777)
);

const mat3 ACESOutputMat = mat3(
    vec3(1.60475, -0.10208, -0.00327),
    vec3(-0.53108, 1.10813, -0.07276),
    vec3(-0.07367, -0.00605, 1.07602)
);

vec3 RRTAndODTFit(vec3 v) {
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 color) {
    color = ACESInputMat * color;

    color = RRTAndODTFit(color);

    color = ACESOutputMat * color;

    color = clamp(color, vec3(0.0), vec3(1.0));

    return color;
}

void main() {
    vec3 hdr = texture(uHDR, inUV).rgb;
    vec3 bloom = texture(uBloom, inUV).rgb;

    vec3 color = mix(hdr, bloom, 0.03);

    outSDR = ACESFitted(color);
}