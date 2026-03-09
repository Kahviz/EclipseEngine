cbuffer LightingCB : register(b1)
{
    float3 lightpos;
    float Brightness;
    float3 WorldPos;
    float lightRange;
};

Texture2D diffuseTexture : register(t0);
SamplerState samplerState : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 color : COLOR0;
    float brightness : TEXCOORD2;
    float2 tex : TEXCOORD3;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texColor = diffuseTexture.Sample(samplerState, input.tex);

    float3 lightDir = normalize(float3(1, 1, -1));
    float3 normal = normalize(input.normal);

    float diffuse = max(0.3f, dot(normal, lightDir));

    float3 finalColor = texColor.rgb * diffuse * (input.brightness / 2);

    return float4(finalColor, texColor.a);
}