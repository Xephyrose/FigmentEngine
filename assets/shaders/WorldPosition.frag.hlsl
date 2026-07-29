struct PSInput {
     float2 uv : TEXCOORD0;
     float3 worldPos : TEXCOORD1;
     float3 worldNormal : TEXCOORD2;
     float3 worldTangent : TEXCOORD3;
     float3 worldBitangent : TEXCOORD4;
     float4 shadowCoord : TEXCOORD5;
};

float4 main(PSInput input) : SV_TARGET {
    return float4(input.worldPos, 1.0);
}