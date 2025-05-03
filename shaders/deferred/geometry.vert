#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUV0;
layout(location = 4) in vec2 inUV1;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outTangent;
layout(location = 3) out vec2 outUV0;
layout(location = 4) out vec2 outUV1;

layout(set = 1, binding = 0) uniform Object {
    mat4 modelMat;
    mat3 normalMat;
    mat4 modelViewProjectionMat;
} uObject;

void main() {
    outPosition = (uObject.modelMat * vec4(inPosition, 1.0)).xyz;
    outNormal = uObject.normalMat * inNormal;
    outTangent = uObject.normalMat * inTangent;
    outUV0 = inUV0;
    outUV1 = inUV1;

    gl_Position = uObject.modelViewProjectionMat * vec4(inPosition, 1.0);
}
