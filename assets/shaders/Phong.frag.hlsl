#include "assets/shaders/includes/Lights.hlsl"

cbuffer PushConstants : register(b0, space3)
{
    float3 viewPos;
}

struct PSInput {
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
};

StructuredBuffer<PointLight> pointLights : register(t0, space2);

float4 main(PSInput input) : SV_TARGET {
    return float4(CalcPointLight(pointLights[0], input.worldNormal, input.worldPos, float3(1.0, 1.0, 1.0), float3(1.0, 1.0, 1.0), CalcPhongSpecular(normalize(pointLights[0].position.xyz - input.worldPos), input.worldNormal, normalize(viewPos - input.worldPos), 64)) + 0.1f, 1.0);
}
