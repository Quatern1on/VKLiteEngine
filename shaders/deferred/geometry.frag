#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUV0;
layout(location = 4) in vec2 inUV1;

layout(location = 0) out vec4 outMain;
layout(location = 1) out vec4 outAlbedo;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec2 outMetallicRoughness;

layout(set = 0, binding = 0) uniform Material {
    vec3 albedo;
    vec3 emissive;
    float metallic;
    float roughness;
} uMaterial;

layout(set = 0, binding = 1) uniform sampler2D uAlbedoTexture;
layout(set = 0, binding = 2) uniform sampler2D uEmissiveTexture;
layout(set = 0, binding = 3) uniform sampler2D uNormalTexture;
layout(set = 0, binding = 4) uniform sampler2D uMetallicTexture;
layout(set = 0, binding = 5) uniform sampler2D uRoughnessTexture;
layout(set = 0, binding = 6) uniform sampler2D uAOTexture;

void main() {
    outMain = vec4(uMaterial.emissive * texture(uEmissiveTexture, inUV0).rgb, 1.0);
    outAlbedo = vec4(uMaterial.albedo * texture(uAlbedoTexture, inUV0).rgb, 1.0);
    outMetallicRoughness = vec2(uMaterial.metallic, uMaterial.roughness);
    outMetallicRoughness.r *= texture(uMetallicTexture, inUV0).r;
    outMetallicRoughness.g *= texture(uRoughnessTexture, inUV0).r;

    //Calculate normals
    vec3 N = normalize(inNormal);
    vec3 T = normalize(inTangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    vec3 textureNormal = texture(uNormalTexture, inUV0).rgb;
    textureNormal = normalize(textureNormal * 2.0 - 1.0);
    textureNormal.y *= -1.0;
    vec3 normal = TBN * textureNormal;

    outNormal = vec4(normal * 0.5 + 0.5, texture(uAOTexture, inUV0).r);
}
