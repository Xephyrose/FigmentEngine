cbuffer TransformUBO : register(b0, space1)
{
    float4x4 ModelViewProjection;
    float4x4 Model;
    float4x4 NormalMatrix;
};

struct VSInput
{
    float3 pos : POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float4 tangent : TANGENT0;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
    float3 worldTangent : TEXCOORD3;
    float3 worldBitangent : TEXCOORD4;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.pos = mul(ModelViewProjection, float4(input.pos, 1.0));
    output.uv = input.uv;
    output.worldPos = mul(Model, float4(input.pos, 1.0)).xyz;

    output.worldNormal = normalize(mul((float3x3)NormalMatrix, input.normal));
    output.worldTangent = normalize(mul((float3x3)NormalMatrix, input.tangent));
    output.worldBitangent = normalize(cross(output.worldNormal, output.worldTangent) * input.tangent.w); // handedness stored in tangent.w

    return output;
}