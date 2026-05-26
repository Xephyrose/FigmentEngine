Texture2D g_texture : register(t0, space2);
SamplerState g_sampler : register(s0, space2);

struct PSInput
{
    float2 uv : TEXCOORD1;
};

cbuffer PushConstants : register(b0, space1) {
    float4 color;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = g_texture.Sample(g_sampler, input.uv);
    return texColor;// * color;
}