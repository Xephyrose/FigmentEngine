Texture2D HeightMap : register(t0, space0);
SamplerState HeightSampler : register(s0, space0);

cbuffer TransformUBO : register(b0, space1)
{
    float4x4 ModelViewProjection;
    float4x4 Model;
    float4x4 NormalMatrix;
    float4x4 LightViewProjection;
    float HeightScale;
    float TerrainSizeX;
    float TerrainSizeZ;
    float padding;
}

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
    float4 shadowCoord : TEXCOORD5;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float height = HeightMap.SampleLevel(HeightSampler, input.uv, 0).r;

    float3 displacedPos = input.pos;
    displacedPos.y += height * HeightScale;

    output.pos = mul(ModelViewProjection, float4(displacedPos, 1.0));
    output.uv = input.uv;
    output.worldPos = mul(Model, float4(displacedPos, 1.0)).xyz;

    output.worldNormal = normalize(mul((float3x3)NormalMatrix, input.normal));
    output.worldTangent = normalize(mul((float3x3)NormalMatrix, input.tangent.xyz));
    output.worldBitangent = normalize(cross(output.worldNormal, output.worldTangent) * input.tangent.w);

    float4 shadowPos = mul(LightViewProjection, mul(Model, float4(displacedPos, 1.0)));
    shadowPos.y = -shadowPos.y;
    output.shadowCoord = shadowPos;

    return output;
}