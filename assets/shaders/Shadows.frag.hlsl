Texture2D shadowMap : register(t0, space2);
SamplerComparisonState shadowSampler : register(s0, space2);

struct PSInput {
    float4 shadowCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET {
    float3 projCoords = input.shadowCoord.xyz / input.shadowCoord.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return float4(1,1,1,1); // fully lit
    }

    float closestDepth = shadowMap.SampleCmp(shadowSampler, projCoords.xy, projCoords.z).r;

    float currentDepth = projCoords.z;
    float diff = currentDepth - closestDepth;

    return float4(
        diff * 1000.0 + 0.5,
        diff * 1000.0 + 0.5,
        diff * 1000.0 + 0.5,
        1.0);
}
