cbuffer TransformUBO : register(b0, space1) {
    float4x4 LightViewProjection;
}

struct VSInput {
    float3 position : POSITION0;
};

struct VSOutput {
    float4 position : SV_POSITION;
};

float4 main(VSInput input) : SV_POSITION {
    float4 pos = mul(LightViewProjection, float4(input.position, 1.0));
    pos.z = 0.5f * pos.w;  // Force depth to 0.5
    return pos;
}