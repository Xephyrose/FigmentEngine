Texture2D MyTexture : register(t0, space2);
SamplerState MySampler : register(s0, space2);

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD1;
};

float4 main(PSInput input) : SV_TARGET
{
    return MyTexture.Sample(MySampler, input.UV);
}