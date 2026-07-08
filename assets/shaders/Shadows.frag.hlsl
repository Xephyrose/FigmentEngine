Texture2D shadowMap : register(t0, space2);
SamplerState shadowSampler : register(s0, space2);

struct PSInput {
    float4 shadowCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET {
    float3 projCoords = input.shadowCoord.xyz / input.shadowCoord.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;
    float closestDepth = shadowMap.Sample(shadowSampler, projCoords.xy).r;

    float diff = currentDepth - closestDepth;

    return float4(
        diff * 100.0 + 0.5,
        diff * 100.0 + 0.5,
        diff * 100.0 + 0.5,
        1.0);
}
