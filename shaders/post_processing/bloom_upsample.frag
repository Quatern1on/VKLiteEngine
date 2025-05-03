#version 460

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec3 outDownsample;

layout(set = 0, binding = 0) uniform sampler2D uTexture;

void main() {
    vec2 textureTexelSize = 1.0 / textureSize(uTexture, 0);
    float x = textureTexelSize.x;
    float y = textureTexelSize.y;

    // a - b - c
    // d - e - f
    // g - h - i
    vec3 a = texture(uTexture, vec2(inUV.x - x, inUV.y + y)).rgb;
    vec3 b = texture(uTexture, vec2(inUV.x,     inUV.y + y)).rgb;
    vec3 c = texture(uTexture, vec2(inUV.x + x, inUV.y + y)).rgb;

    vec3 d = texture(uTexture, vec2(inUV.x - x, inUV.y)).rgb;
    vec3 e = texture(uTexture, vec2(inUV.x,     inUV.y)).rgb;
    vec3 f = texture(uTexture, vec2(inUV.x + x, inUV.y)).rgb;

    vec3 g = texture(uTexture, vec2(inUV.x - x, inUV.y - y)).rgb;
    vec3 h = texture(uTexture, vec2(inUV.x,     inUV.y - y)).rgb;
    vec3 i = texture(uTexture, vec2(inUV.x + x, inUV.y - y)).rgb;

    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    outDownsample = e * 4.0;
    outDownsample += (b + d + f + h) * 2.0;
    outDownsample += (a + c + g + i);
    outDownsample *= 1.0 / 16.0;
}
