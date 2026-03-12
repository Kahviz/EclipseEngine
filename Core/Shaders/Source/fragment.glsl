#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 color;
    float usesTexture;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    if (ubo.usesTexture > 0.5) {
        // Käytä tekstuuria
        outColor = texture(texSampler, fragUV);
    } else {
        // Käytä vain väriä
        outColor = vec4(fragColor, 1.0);
    }
}