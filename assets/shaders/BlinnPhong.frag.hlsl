#include "assets/shaders/includes/Lights.hlsl"

Texture2D g_shadow_map : register(t0, space2);
SamplerComparisonState g_shadow_sampler : register(s0, space2);

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
    float4 shadowCoord : TEXCOORD3;
};

StructuredBuffer<PointLight> pointLights : register(t1, space2);
StructuredBuffer<DirectionalLight> directionalLights : register(t2, space2);
StructuredBuffer<SpotLight> spotLights : register(t3, space2);

float4 main(PSInput input) : SV_TARGET {
    float3 result = float3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < num_point_lights; i++) {
        result += CalcPointLight(pointLights[i], input.worldNormal, input.worldPos, float3(1.0, 1.0, 1.0), CalcBlinnPhongSpecular(normalize(pointLights[i].position.xyz - input.worldPos), input.worldNormal, normalize(viewPos - input.worldPos), 64));
    }
    for(int i = 0; i < num_dir_lights; i++) {
       DirectionalLight light = directionalLights[i];
       light.direction.w = CalcDirectionalLightShadows(light, g_shadow_map, g_shadow_sampler, input.shadowCoord, input.worldNormal, 1);
       result += CalcDirectionalLight(light, input.worldNormal, float3(1.0, 1.0, 1.0), CalcBlinnPhongSpecular(normalize(-directionalLights[i].direction.xyz), input.worldNormal, normalize(viewPos - input.worldPos), 64));
    }
    for(int i = 0; i < num_spot_lights; i++) {
        result += CalcSpotLight(spotLights[i], input.worldNormal, input.worldPos, float3(1.0, 1.0, 1.0), CalcPhongSpecular(normalize(spotLights[i].position.xyz - input.worldPos), input.worldNormal, normalize(viewPos - input.worldPos), 64));
    }

    return float4(0.3 + result, 1);
}
