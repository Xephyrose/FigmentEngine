Texture2D shadowMap : register(t0, space2);
SamplerState shadowSampler : register(s0, space2);

struct PSInput {
    float4 shadowCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET {
    float3 projCoords = input.shadowCoord.xyz / input.shadowCoord.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    projCoords = saturate(projCoords);

    float depth = shadowMap.Sample(shadowSampler, projCoords.xy).r;
    return float4(depth, depth, depth, 1.0);
}