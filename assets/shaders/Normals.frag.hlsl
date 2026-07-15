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
    float3 normalColor = input.worldNormal * 0.5 + 0.5;
    return float4(normalColor, 1.0);
}