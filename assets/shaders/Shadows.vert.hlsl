cbuffer TransformUBO : register(b0, space1) {
    float4x4 ModelViewProjection;
    float4x4 Model;
    float4x4 LightViewProjection;
}

struct VSInput {
    float3 position : POSITION0;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float4 shadowCoord : TEXCOORD0;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = mul(ModelViewProjection, float4(input.position, 1.0));
    output.shadowCoord = mul(LightViewProjection, mul(Model, float4(input.position, 1.0)));
    return output;
}