// first float4 is color, second is position, in an alternating pattern, e.g:

//pointLights[0] = position0
//pointLights[1] = color0
//pointLights[2] = position1
//pointLights[3] = color1

StructuredBuffer<float4> pointLights : register(t0, space2);

struct PSInput {
    float3 worldPos : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET {
//    return pointLights[0];
//    return float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 position = pointLights[1];
    float4 color = pointLights[0];

    // Lighting calculation using position and color
    float3 lightDir = -position.xyz - input.worldPos;
    float dist = length(lightDir);
    float att = 1.0 / (1.0 + dist * dist);
    float3 lit = att * color.rgb * color.w;

    return float4(lit, 1.0);
}
