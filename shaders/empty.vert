#version 460

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outPosition;

layout(set = 0, binding = 0) uniform Empty {
    vec3 empty;
} uEmpty;

void main() {
}
