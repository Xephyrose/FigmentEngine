cbuffer TransformUBO : register(b0, space1)
{
    float4x4 ModelViewProjection;
    float4x4 Model;
};

struct VSInput
{
    float3 pos : POSITION0;
    float2 uv : TEXCOORD0;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.pos = mul(ModelViewProjection, float4(input.pos, 1.0));
    output.uv = input.uv;
    output.worldPos = mul(Model, float4(input.pos, 1.0)).xyz;
    return output;
}