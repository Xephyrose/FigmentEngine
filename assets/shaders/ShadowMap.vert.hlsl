cbuffer TransformUBO : register(b0, space1) {
    float4x4 LightViewProjection;
};

struct VSInput {
    float3 position : POSITION0;
};

struct VSOutput {
    float4 position : SV_POSITION;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = mul(LightViewProjection, float4(input.position, 1.0));
    return output;
}
