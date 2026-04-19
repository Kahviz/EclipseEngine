#version 450

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 color;
    float usesTexture;
    mat4 lightSpaceMatrix;  // lisätty
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec4 fragPosLightSpace;  // lisätty

void main() {
    fragColor = inColor;
    fragUV = inUV;
    fragNormal = normalize(mat3(ubo.model) * inNormal);
    fragPosLightSpace = ubo.lightSpaceMatrix * ubo.model * vec4(inPos, 1.0);  // lisätty
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPos, 1.0);
}