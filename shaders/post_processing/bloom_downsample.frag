#version 460

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec3 outDownsample;

layout(set = 0, binding = 0) uniform sampler2D uTexture;

void main() {
    vec2 textureTexelSize = 1.0 / textureSize(uTexture, 0);
    float x = textureTexelSize.x;
    float y = textureTexelSize.y;

    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    vec3 a = texture(uTexture, vec2(inUV.x - 2 * x, inUV.y + 2 * y)).rgb;
    vec3 b = texture(uTexture, vec2(inUV.x, inUV.y + 2 * y)).rgb;
    vec3 c = texture(uTexture, vec2(inUV.x + 2 * x, inUV.y + 2 * y)).rgb;

    vec3 d = texture(uTexture, vec2(inUV.x - 2 * x, inUV.y)).rgb;
    vec3 e = texture(uTexture, vec2(inUV.x, inUV.y)).rgb;
    vec3 f = texture(uTexture, vec2(inUV.x + 2 * x, inUV.y)).rgb;

    vec3 g = texture(uTexture, vec2(inUV.x - 2 * x, inUV.y - 2 * y)).rgb;
    vec3 h = texture(uTexture, vec2(inUV.x, inUV.y - 2 * y)).rgb;
    vec3 i = texture(uTexture, vec2(inUV.x + 2 * x, inUV.y - 2 * y)).rgb;

    vec3 j = texture(uTexture, vec2(inUV.x - x, inUV.y + y)).rgb;
    vec3 k = texture(uTexture, vec2(inUV.x + x, inUV.y + y)).rgb;
    vec3 l = texture(uTexture, vec2(inUV.x - x, inUV.y - y)).rgb;
    vec3 m = texture(uTexture, vec2(inUV.x + x, inUV.y - y)).rgb;

    outDownsample = e * 0.125;
    outDownsample += (a + c + g + i) * 0.03125;
    outDownsample += (b + d + f + h) * 0.0625;
    outDownsample += (j + k + l + m) * 0.125;
}
