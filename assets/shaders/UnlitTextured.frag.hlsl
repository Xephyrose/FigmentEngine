Texture2D g_texture : register(t0, space2);
SamplerState g_sampler : register(s0, space2);

cbuffer PushConstants : register(b0, space3)
{
    float4 color;
    bool    useTexture;
};

struct PSInput {
    float2 uv : TEXCOORD1;
};

float4 main(PSInput input) : SV_TARGET
{
    if (useTexture == true) {
        float4 texColor = g_texture.Sample(g_sampler, input.uv);
        return texColor * color;
    }
    else {
        return color;
    }
}