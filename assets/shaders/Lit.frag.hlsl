// first float4 is color, second is position, in an alternating pattern, e.g:

//pointLights[0] = position0
//pointLights[1] = color0
//pointLights[2] = position1
//pointLights[3] = color1

StructuredBuffer<float4> pointLights : register(t0, space2);

struct PSInput {
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

float4 main(PSInput input) : SV_TARGET {
    float4 color = pointLights[0];
    float4 position = pointLights[1];

    float3 lightDir = position.xzy - input.worldPos;
    float dist = length(lightDir);
    float att = 1.0 / (1.0 + dist * dist);
    float3 lit = att * color.rgb * color.w;

    return float4(lit, 1.0);
}
