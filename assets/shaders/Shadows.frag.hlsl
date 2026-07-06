Texture2D shadowMap : register(t0, space2);
SamplerState shadowSampler : register(s0, space2);

struct PSInput {
    float4 shadowCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET {
    float3 projCoords = input.shadowCoord.xyz / input.shadowCoord.w;
    projCoords = projCoords * 0.5 + 0.5;

    float depth = shadowMap.Sample(shadowSampler, projCoords.xy).r;

//    float currentDepth = projCoords.z;
//    float shadow = 1 - (currentDepth > depth  ? 1.0 : 0.0);
//    return float4(shadow, shadow, shadow, 1.0f);

    return float4(depth, depth, depth, 1.0);
}