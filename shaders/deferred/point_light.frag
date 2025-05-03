#version 460

#define PI 3.1415926538

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inAlbedoAo;
layout(input_attachment_index = 0, set = 0, binding = 1) uniform subpassInput inNormal;
layout(input_attachment_index = 0, set = 0, binding = 2) uniform subpassInput inMetallicRoughness;
layout(input_attachment_index = 0, set = 0, binding = 3) uniform subpassInput inDepth;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform Frame {
    vec3 cameraOrigin;
    mat4 inverseTransform;
    vec2 screenSize;
} uFrame;

layout(set = 2, binding = 1) uniform PointLight {
    vec3 position;
    vec3 intensity;
    float radius;
} uLight;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    //Check clamp
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom + 0.00001;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main() {
    vec4 tempPosition = vec4(gl_FragCoord.xy, subpassLoad(inDepth).x, 1.0);
    tempPosition.xy /= uFrame.screenSize;
    tempPosition.xy *= 2.0f;
    tempPosition.xy -= 1.0f;
    tempPosition = uFrame.inverseTransform * tempPosition;
    tempPosition /= tempPosition.w;

    vec3 albedo = subpassLoad(inAlbedoAo).rgb;
    vec3 normal = subpassLoad(inNormal).rgb * 2.0 - 1.0;
    vec2 metallicRoughness = subpassLoad(inMetallicRoughness).rg;
    vec3 position = tempPosition.xyz;

    vec3 V = normalize(uFrame.cameraOrigin - position);
    vec3 L = normalize(uLight.position - position);
    vec3 H = normalize(V + L);

    float distance    = length(uLight.position - position);

    float attenuation = 1.0 / (distance * distance);

    vec3 radiance     = uLight.intensity * attenuation;

    vec3 F0 = vec3(0.04);
    F0      = mix(F0, albedo, metallicRoughness.x);
    vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NDF = DistributionGGX(normal, H, metallicRoughness.y);
    float G   = GeometrySmith(normal, V, L, metallicRoughness.y);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metallicRoughness.x;

    float NdotL = max(dot(normal, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    outColor = vec4(Lo, 1.0f);
}
