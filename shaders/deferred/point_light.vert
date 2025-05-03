#version 460

layout(location = 0) in vec3 inPosition;

layout(set = 2, binding = 0) uniform Object {
    mat4 modelViewProjectionMat;
} uObject;

void main() {
    gl_Position = uObject.modelViewProjectionMat * vec4(inPosition, 1.0);
}
