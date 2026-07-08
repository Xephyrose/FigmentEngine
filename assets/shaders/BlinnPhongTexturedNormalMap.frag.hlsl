#include "assets/shaders/includes/Lights.hlsl"

Texture2D g_albedo : register(t0, space2);
Texture2D g_ambient : register(t1, space2);
Texture2D g_specular : register(t2, space2);
Texture2D g_normal_map : register(t3, space2);
Texture2D g_shadow_map : register(t4, space2);
SamplerState g_sampler0 : register(s0, space2);
SamplerState g_sampler1 : register(s1, space2);
SamplerState g_sampler2 : register(s2, space2);
SamplerState g_sampler3 : register(s3, space2);
SamplerComparisonState g_shadow_sampler : register(s4, space2);

cbuffer PushConstants : register(b0, space3)
{
    float3  viewPos;
    float   shininess;
    float4  colorAlbedo;
    bool    useAlbedoTexture;
    float3  colorAmbient;
    bool    useAmbientTexture;
    float3  colorSpecular;
    bool    useSpecularTexture;
    int     num_point_lights;
    int     num_dir_lights;
    int     num_spot_lights;
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
    float3 sampledNormal = g_normal_map.Sample(g_sampler3, input.uv).rgb;
    float3 tangentNormal = sampledNormal * 2.0 - 1.0;

    float3 N = normalize(input.worldNormal);
    float3 T = normalize(input.worldTangent);
    float3 B = normalize(input.worldBitangent);

    T = normalize(T - dot(T, N) * N);
    B = cross(N, T);

    float3 worldNormal = normalize(T * tangentNormal.x + B * tangentNormal.y + N * tangentNormal.z);

    float4 calcAlbedo;
    if (useAlbedoTexture == true) {
        float4 texColor = g_albedo.Sample(g_sampler0, input.uv);
        calcAlbedo = texColor * colorAlbedo;
    }
    else {
        calcAlbedo = colorAlbedo;
    }

    float3 calcAmbient;
    if (useAmbientTexture == true) {
        float3 texColor = g_ambient.Sample(g_sampler1, input.uv).xyz;
        calcAmbient = texColor * colorAmbient;
    }
    else {
        calcAmbient = colorAmbient;
    }

    float3 calcSpecular;
    if (useSpecularTexture == true) {
        float3 texColor = g_specular.Sample(g_sampler2, input.uv).xyz;
        calcSpecular = texColor * colorSpecular;
    }
    else {
        calcSpecular = colorSpecular;
    }

    float3 result = float3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < num_point_lights; i++) {
        result += CalcPointLight(pointLights[i], worldNormal, input.worldPos, calcAlbedo.xyz, calcSpecular, CalcBlinnPhongSpecular(normalize(pointLights[i].position.xyz - input.worldPos), worldNormal, normalize(viewPos - input.worldPos), shininess));
    }
    for(int i = 0; i < num_dir_lights; i++) {
        result += CalcDirectionalLight(directionalLights[i], worldNormal, calcAlbedo.xyz, calcSpecular, CalcBlinnPhongSpecular(normalize(-directionalLights[i].direction.xyz), worldNormal, normalize(viewPos - input.worldPos), shininess));
    }
    for(int i = 0; i < num_spot_lights; i++) {
        result += CalcSpotLight(spotLights[i], worldNormal, input.worldPos, float3(1.0, 1.0, 1.0), float3(1.0, 1.0, 1.0), CalcPhongSpecular(normalize(spotLights[i].position.xyz - input.worldPos), worldNormal, normalize(viewPos - input.worldPos), 64));
    }

    float3 projCoords = input.shadowCoord.xyz / input.shadowCoord.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
            projCoords.y < 0.0 || projCoords.y > 1.0 ||
            projCoords.z < 0.0 || projCoords.z > 1.0)
        {
            return float4(result + calcAmbient, calcAlbedo.w);
        } else {
            float closestDepth = g_shadow_map.SampleCmp(g_shadow_sampler, projCoords.xy, projCoords.z).r;

            float currentDepth = projCoords.z;

            float3 lightDir = normalize(-directionalLights[0].direction.xyz);

            float bias = max(0.0001 * (1.0 - saturate(dot(normalize(input.worldNormal), lightDir))), 0.00001);

            float shadow = currentDepth - bias > closestDepth  ? 0.3 : 1.0;

            float3 lighting = (calcAmbient + shadow * result);
            return float4(lighting, calcAlbedo.w);
        }
}
