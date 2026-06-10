struct PointLight {
    float4 position;
    float4 color;
};
StructuredBuffer<PointLight> pointLights : register(t0, space2);

float4 main() : SV_TARGET {
//    PointLight light = pointLights[0];
//    return float4(0.0, light.position.x, 0.0, 1.0);

    return pointLights[0].color;

//    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
