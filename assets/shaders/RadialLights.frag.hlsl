StructuredBuffer<float4> pointLights : register(t0, space2);

struct PSInput {
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
    float3 worldTangent : TEXCOORD3;
    float3 worldBitangent : TEXCOORD4;
    float4 shadowCoord : TEXCOORD5;
};

float4 main(PSInput input) : SV_TARGET {
    float3 lightpos = float3(pointLights[1].x, pointLights[1].y, pointLights[1].z);
    float3 lightDir = normalize(lightpos - input.worldPos);
    return float4(lightDir, 1.0);
}