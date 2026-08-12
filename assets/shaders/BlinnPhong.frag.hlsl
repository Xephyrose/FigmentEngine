#include "assets/shaders/includes/Lights.hlsli"

Texture2D g_shadow_map : register(t0, space2);
SamplerComparisonState g_shadow_sampler : register(s0, space2);

cbuffer PushConstants : register(b0, space3)
{
    float4 viewPos;
    float4 params; // num_point_lights, num_dir_lights, num_spot_lights, padding
}

struct PSInput {
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
    float3 worldTangent : TEXCOORD3;
    float3 worldBitangent : TEXCOORD4;
    float4 shadowCoord : TEXCOORD5;
};

StructuredBuffer<PointLight> pointLights : register(t1, space2);
StructuredBuffer<DirectionalLight> directionalLights : register(t2, space2);
StructuredBuffer<SpotLight> spotLights : register(t3, space2);

float4 main(PSInput input) : SV_TARGET {
    float4 calcAlbedo = 1.0;
    float3 calcAmbient = 0.3;
    float3 calcSpecular = 1.0;
    int shininess = 64;

    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);

    float3 pos = normalize(viewPos.xyz - input.worldPos);

    for(int i = 0; i < params.x; i++) {
        diffuse += CalcPointLightDiffuse(pointLights[i], input.worldNormal, input.worldPos);
        specular += CalcPointLightSpecular(pointLights[i], input.worldPos, calcSpecular, CalcBlinnPhongSpecular(normalize(pointLights[i].position.xyz - input.worldPos), input.worldNormal, pos, shininess));
    }
    for(int i = 0; i < params.y; i++) {
        DirectionalLight light = directionalLights[i];
        light.direction.w = CalcDirectionalLightShadows(light, g_shadow_map, g_shadow_sampler, input.shadowCoord, input.worldNormal, 1);
        diffuse += CalcDirectionalLightDiffuse(light, input.worldNormal);
        specular += CalcDirectionalLightSpecular(light, calcSpecular, CalcBlinnPhongSpecular(normalize(-directionalLights[i].direction.xyz), input.worldNormal, pos, shininess));
    }
    for(int i = 0; i < params.z; i++) {
        diffuse += CalcSpotLightDiffuse(spotLights[i], input.worldNormal, input.worldPos);
        specular += CalcSpotLightSpecular(spotLights[i], input.worldPos, calcSpecular, CalcBlinnPhongSpecular(normalize(spotLights[i].position.xyz - input.worldPos), input.worldNormal, pos, shininess));
    }

    float3 lighting = (calcAmbient + diffuse) * calcAlbedo.xyz + specular;
    return float4(lighting, calcAlbedo.w);
}
