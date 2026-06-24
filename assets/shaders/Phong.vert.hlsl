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
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 tangentLightPos : TEXCOORD1;
    float3 tangentViewPos : TEXCOORD2;
    float3 tangentFragPos : TEXCOORD3;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.pos = mul(ModelViewProjection, float4(input.pos, 1.0));
    output.uv = input.uv;

    float3 worldPos = mul(Model, float4(input.pos, 1.0)).xyz;
    float3 worldNormal = normalize(mul(NormalMatrix, input.normal));
    float3 worldTangent = normalize(mul(NormalMatrix, input.tangent));
    float3 worldBitangent = normalize(cross(worldNormal, worldTangent));

    // Build TBN matrix (transpose = inverse for orthonormal basis)
    float3x3 TBN = float3x3(worldTangent, worldBitangent, worldNormal);
    TBN = transpose(TBN);  // Convert world → tangent space

    // Transform light, view, and fragment positions to tangent space
    output.tangentLightPos = mul(TBN, lightPos);
    output.tangentViewPos = mul(TBN, viewPos);
    output.tangentFragPos = mul(TBN, worldPos);

    return output;
}