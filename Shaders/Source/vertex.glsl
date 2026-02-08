#version 450

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 color;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    fragColor = ubo.color;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPos, 1.0);
}