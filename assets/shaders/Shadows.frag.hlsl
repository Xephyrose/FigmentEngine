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

//    return float4(projCoords.xy, 0, 1); // renders a gradient of red and green, but I think it looks a little different now

//    return float4(projCoords.z.xxx, 1);  // NOW RENDERS A GREYSCALE GRADIENT ALL ACROSS MESH

    return float4(depth, depth, depth, 1.0); // renders depth in some areas, shadows in some too. I don't really understand exactly what it's trying to render

//return float4(input.shadowCoord.z / 100.0, 0, 0, 1); // still renders black?

//    float z = input.shadowCoord.z / input.shadowCoord.w;
//    return float4(z > 0 ? 1 : 0, z < 0 ? 1 : 0, 0, 1); // all red?

//float z = input.shadowCoord.z / input.shadowCoord.w;
//return float4(z * 0.01 + 0.5, z * 0.01 + 0.5, z * 0.01 + 0.5, 1); // all grey?
}