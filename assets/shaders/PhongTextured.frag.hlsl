#include "assets/shaders/includes/Lights.hlsli"

Texture2D g_albedo : register(t0, space2);
Texture2D g_ambient : register(t1, space2);
Texture2D g_specular : register(t2, space2);
Texture2D g_normal_map : register(t3, space2);
Texture2D g_shadow_map : register(t4, space2);
SamplerState g_sampler_albedo : register(s0, space2);
SamplerState g_sampler_ambient : register(s1, space2);
SamplerState g_sampler_specular : register(s2, space2);
SamplerState g_sampler_normal_map : register(s3, space2);
SamplerComparisonState g_shadow_sampler : register(s4, space2);

cbuffer PushConstants : register(b0, space3)
{
    float4 viewPos;
    float4 colorAlbedo;
    uint4 texturesUsed; // albedo, ambient, specular, normal
    float4 colorAmbient;
    float4 colorSpecular;
    float4 lightNums; // num_point_lights, num_dir_lights, num_spot_lights
    float4 params; // shininess
}

struct PSInput {
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
    float3 worldTangent : TEXCOORD3;
    float3 worldBitangent : TEXCOORD4;
    float4 shadowCoord : TEXCOORD5;
};

StructuredBuffer<PointLight> pointLights : register(t5, space2);
StructuredBuffer<DirectionalLight> directionalLights : register(t6, space2);
StructuredBuffer<SpotLight> spotLights : register(t7, space2);

float4 main(PSInput input) : SV_TARGET {
    float3 worldNormal = normalize(input.worldNormal);
    if (texturesUsed.w == true) {
        float3 sampledNormal = g_normal_map.Sample(g_sampler_normal_map, input.uv).rgb;
        float3 tangentNormal = sampledNormal * 2.0 - 1.0;

        float3 N = normalize(input.worldNormal);
        float3 T = normalize(input.worldTangent);
        float3 B = normalize(input.worldBitangent);

        T = normalize(T - dot(T, N) * N);
        B = cross(N, T);

        worldNormal = normalize(T * tangentNormal.x + B * tangentNormal.y + N * tangentNormal.z);
    }

    float4 calcAlbedo = colorAlbedo;
    if (texturesUsed.x == true) {
        float4 texColor = g_albedo.Sample(g_sampler_albedo, input.uv);
        calcAlbedo = texColor * colorAlbedo;
    }

    float3 calcAmbient = colorAmbient.xyz;
    if (texturesUsed.y == true) {
        float3 texColor = g_ambient.Sample(g_sampler_ambient, input.uv).xyz;
        calcAmbient = texColor * colorAmbient.xyz;
    }

    float3 calcSpecular = colorSpecular.xyz;
    if (texturesUsed.z == true) {
        float3 texColor = g_specular.Sample(g_sampler_specular, input.uv).xyz;
        calcSpecular = texColor * colorSpecular.xyz;
    }

    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < lightNums.x; i++) {
        diffuse += CalcPointLightDiffuse(pointLights[i], worldNormal, input.worldPos);
        specular += CalcPointLightSpecular(pointLights[i], worldNormal, calcSpecular, CalcPhongSpecular(normalize(pointLights[i].position.xyz - input.worldPos), worldNormal, normalize(viewPos.xyz - input.worldPos), params.x));
    }
    for(int i = 0; i < lightNums.y; i++) {
        DirectionalLight light = directionalLights[i];
        light.direction.w = CalcDirectionalLightShadows(light, g_shadow_map, g_shadow_sampler, input.shadowCoord, worldNormal, 1);
        diffuse += CalcDirectionalLightDiffuse(light, worldNormal);
        specular += CalcDirectionalLightSpecular(light, calcSpecular, CalcPhongSpecular(normalize(-directionalLights[i].direction.xyz), worldNormal, normalize(viewPos.xyz - input.worldPos), params.x));
    }
    for(int i = 0; i < lightNums.z; i++) {
        diffuse += CalcSpotLightDiffuse(spotLights[i], worldNormal, input.worldPos);
        specular += CalcSpotLightSpecular(spotLights[i], input.worldPos, calcSpecular, CalcPhongSpecular(normalize(spotLights[i].position.xyz - input.worldPos), worldNormal, normalize(viewPos.xyz - input.worldPos), params.x));
    }

    float3 lighting = (calcAmbient + diffuse) * calcAlbedo.xyz + specular;
    return float4(lighting, calcAlbedo.w);
}
