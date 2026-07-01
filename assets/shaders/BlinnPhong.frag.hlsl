#include "assets/shaders/includes/Lights.hlsl"

cbuffer PushConstants : register(b0, space3)
{
    float3  viewPos;
    int     num_point_lights;
    int     num_dir_lights;
    int     num_spot_lights;
}

struct PSInput {
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
};

StructuredBuffer<PointLight> pointLights : register(t0, space2);
StructuredBuffer<DirectionalLight> directionalLights : register(t1, space2);
//StructuredBuffer<SpotLight> spotLights : register(t2, space2);

float4 main(PSInput input) : SV_TARGET {
    float3 result = float3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < num_point_lights; i++) {
        result += CalcPointLight(pointLights[i], input.worldNormal, input.worldPos, float3(1.0, 1.0, 1.0), float3(1.0, 1.0, 1.0), CalcBlinnPhongSpecular(normalize(pointLights[i].position.xyz - input.worldPos), input.worldNormal, normalize(viewPos - input.worldPos), 64));
    }
    for(int i = 0; i < num_dir_lights; i++) {
        result += CalcDirectionalLight(directionalLights[i], input.worldNormal, float3(1.0, 1.0, 1.0), float3(1.0, 1.0, 1.0), CalcBlinnPhongSpecular(normalize(-directionalLights[i].direction.xyz), input.worldNormal, normalize(viewPos - input.worldPos), 64));
    }
    return float4(result + 0.1, 1.0f);
}
