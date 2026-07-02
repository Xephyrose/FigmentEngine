Texture2D shadowMap : register(t0, space2);
SamplerComparisonState shadowSampler : register(s0, space2);

struct PSInput {
    float4 shadowCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET {
    float3 projCoords = input.shadowCoord.xyz / input.shadowCoord.w;
    projCoords = projCoords * 0.5 + 0.5;
    projCoords = saturate(projCoords);

    float shadow = shadowMap.SampleCmp(shadowSampler, projCoords.xy, projCoords.z);
    return float4(shadow, shadow, shadow, 1.0);
}