Texture2D shadowMap : register(t0, space2);
SamplerComparisonState shadowSampler : register(s0, space2);

struct PSInput {
    float4 shadowCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET {
    float3 projCoords = input.shadowCoord.xyz / input.shadowCoord.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0)
    {
        return float4(0,1,1,1);
    }
    if (projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return float4(1,0,1,1);
    }
    if (projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return float4(1,1,0,1);
    }

    float closestDepth = shadowMap.SampleCmp(shadowSampler, projCoords.xy, projCoords.z).r;

    float currentDepth = projCoords.z;
    float bias = 0.001; // lower values can cause something akin to AO. this could be good or bad. higher values may produce peter panning (shadows disconnected from source mesh).
//    float shadow = 0.0;

//    uint width;
//    uint height;
//
//    shadowMap.GetDimensions(width, height);
//
//    float2 texelSize = float2(1.0, 1.0) / float2(width, height);
//    for(int x = -1; x <= 1; ++x)
//    {
//        for(int y = -1; y <= 1; ++y)
//        {
//            float pcfDepth = shadowMap.SampleCmp(shadowSampler, projCoords.xy + float2(x, y) * texelSize, projCoords.z).r;
//            shadow += currentDepth - bias > pcfDepth ? 0.0 : 1.0;
//        }
//    }
//    shadow /= 9.0;

    float shadow = currentDepth - bias > closestDepth  ? 0.0 : 1.0;
    return float4(shadow, shadow, shadow, 1.0);
}
