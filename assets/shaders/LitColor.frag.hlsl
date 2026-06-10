Texture2D g_texture : register(t0, space2);
SamplerState g_sampler : register(s0, space2);

struct PointLight {
    float3 color;
    float3 position;
    float intensity;
};
StructuredBuffer<PointLight> pointLights : register(t0, space2);
cbuffer PushConstants : register(b0, space3) {
    uint lightCount;
};

struct PSInput {
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 uv : TEXCOORD2;
};

float4 main(PSInput input) : SV_TARGET {
    PointLight light = pointLights[0];
    float3 lightDir = light.position.xyz - input.worldPos;
    float dist = length(lightDir);
    float att = 1.0 / (1.0 + dist * dist);
    float3 lit = att * light.color.rgb * light.intensity;
//    return float4(lightCount, 0.0, 0.0, 1.0);
    return float4(lit, 1.0);
}