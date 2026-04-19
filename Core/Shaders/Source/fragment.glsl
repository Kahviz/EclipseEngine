#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragPosLightSpace;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 color;
    float usesTexture;
    mat4 lightSpaceMatrix;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D shadowMap; // TILAPÄISESTI sampler2D debuggaukseen

void main() {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    // Näytä shadow map pienessä nurkassa
    vec2 debugUV = gl_FragCoord.xy / vec2(1280.0, 720.0); // resoluutiosi
    
    if (debugUV.x < 0.25 && debugUV.y < 0.25) {
        // Vasen alakulma = shadow map
        vec2 shadowUV = debugUV * 4.0; // skaalaa 0-1 alueelle
        float depth = texture(shadowMap, shadowUV).r;
        
        // Linearisointi näkyvyyttä varten
        float zNear = 0.1;
        float zFar = 200.0;
        float linearDepth = (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
        
        outColor = vec4(vec3(linearDepth), 1.0);
        return;
    }

    // Normaali valaistus muulle scenelle
    vec3 lightDir = normalize(vec3(0.5, -1.0, 0.5));
    float diff = max(dot(fragNormal, -lightDir), 0.0);
    float ambient = 0.3;
    float lighting = ambient + (1.0 - ambient) * diff;

    vec4 baseColor;
    if (ubo.usesTexture > 0.5) {
        baseColor = texture(texSampler, fragUV);
    } else {
        baseColor = vec4(ubo.color, 1.0);
    }

    outColor = vec4(baseColor.rgb * lighting, baseColor.a);
}