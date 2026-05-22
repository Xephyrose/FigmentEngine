cbuffer TransformUBO : register(b0, space1)
{
    float4x4 ModelViewProjection;
};

struct VSInput
{
    float3 Position : POSITION0;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD1;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Position = mul(ModelViewProjection, float4(input.Position, 1.0));
    output.TexCoord = input.TexCoord;
    return output;
}