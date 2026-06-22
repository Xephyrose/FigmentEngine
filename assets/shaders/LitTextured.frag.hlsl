Texture2D g_texture : register(t0, space2);
SamplerState g_sampler : register(s0, space2);

cbuffer PushConstants : register(b0, space3)
{
    float4 color;
    bool   useTexture;
}

struct PSInput {
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

StructuredBuffer<float4> pointLights : register(t1, space2);

float4 main(PSInput input) : SV_TARGET {
    float4 calc_color;
    if (useTexture == true) {
        float4 texColor = g_texture.Sample(g_sampler, input.uv);
        calc_color = texColor * color;
    }
    else {
        calc_color = color;
    }

    float4 color = pointLights[0];
    float4 position = pointLights[1];

    float3 lightDir = position.xzy - input.worldPos;
    float dist = length(lightDir);
    float att = 1.0 / (1.0 + dist * dist);
    float3 lit = att * color.rgb * color.w;

    return calc_color * float4(lit, 1.0);
}
