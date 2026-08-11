#include "assets/shaders/includes/Lights.hlsli"

Texture2DArray g_albedo : register(t0, space2);
Texture2DArray g_ambient : register(t1, space2);
Texture2DArray g_specular : register(t2, space2);
Texture2DArray g_normal_map : register(t3, space2);
Texture2D g_shadow_map : register(t4, space2);

SamplerState g_sampler_albedo : register(s0, space2);
SamplerState g_sampler_ambient : register(s1, space2);
SamplerState g_sampler_specular : register(s2, space2);
SamplerState g_sampler_normal_map : register(s3, space2);
SamplerComparisonState g_shadow_sampler : register(s4, space2);

cbuffer PushConstants : register(b0, space3)
{
    float4 viewPos;
    float4 params; // num_point_lights, num_dir_lights, num_spot_lights, shininess
    float4 params2; // height count, 3 padding
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
StructuredBuffer<float> splits : register(t8, space2);
StructuredBuffer<uint4> texturesUsed : register(t9, space2); // albedo, ambient, specular, normal
StructuredBuffer<float4> colorAlbedos : register(t10, space2); // rgba
StructuredBuffer<float4> colorAmbients : register(t11, space2); // rgb, padding
StructuredBuffer<float4> colorSpeculars : register(t12, space2); // rgb, padding

float4 getLayerAlbedo(uint layer, PSInput input) {
    float4 calcAlbedo = colorAlbedos[layer];
    if (texturesUsed[layer].x == true) {
        float4 texColor = g_albedo.Sample(g_sampler_albedo, float3(input.uv, layer));
        calcAlbedo = texColor * colorAlbedos[layer];
    }
    return calcAlbedo;
}

float3 getLayerAmbient(uint layer, PSInput input) {
    float3 calcAmbient = colorAmbients[layer].xyz;
    if (texturesUsed.y == true) {
        float3 texColor = g_ambient.Sample(g_sampler_ambient, float3(input.uv, layer)).xyz;
        calcAmbient = texColor * colorAmbients[layer].xyz;
    }
    return calcAmbient;
}

float3 getLayerSpecular(uint layer, PSInput input) {
    float3 calcSpecular = colorSpeculars[layer].xyz;
    if (texturesUsed[layer].z == true) {
        float3 texColor = g_specular.Sample(g_sampler_specular, float3(input.uv, layer)).xyz;
        calcSpecular = texColor * colorSpeculars[layer].xyz;
    }
    return calcSpecular;
}

float3 getLayerNormal(uint layer, PSInput input) {
    float3 worldNormal = normalize(input.worldNormal);
    if (texturesUsed[layer].w == true) {
        float3 sampledNormal = g_normal_map.Sample(g_sampler_normal_map, float3(input.uv, layer)).rgb;
        float3 tangentNormal = sampledNormal * 2.0 - 1.0;

        float3 N = normalize(input.worldNormal);
        float3 T = normalize(input.worldTangent);
        float3 B = normalize(input.worldBitangent);

        T = normalize(T - dot(T, N) * N);
        B = cross(N, T);

        worldNormal = normalize(T * tangentNormal.x + B * tangentNormal.y + N * tangentNormal.z);
    }
    return worldNormal;
}

float4 main(PSInput input) : SV_TARGET {
    float3 worldNormal = getLayerNormal(1, input);
    float4 calcAlbedo = getLayerAlbedo(1, input);
    float3 calcAmbient = getLayerAmbient(1, input);
    float3 calcSpecular = getLayerSpecular(1, input);

    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < params.x; i++) {
        diffuse += CalcPointLightDiffuse(pointLights[i], worldNormal, input.worldPos);
        specular += CalcPointLightSpecular(pointLights[i], worldNormal, calcSpecular, CalcBlinnPhongSpecular(normalize(pointLights[i].position.xyz - input.worldPos), worldNormal, normalize(viewPos.xyz - input.worldPos), params.w));
    }
    for(int i = 0; i < params.y; i++) {
        DirectionalLight light = directionalLights[i];
        light.direction.w = CalcDirectionalLightShadows(light, g_shadow_map, g_shadow_sampler, input.shadowCoord, worldNormal, 1);
        diffuse += CalcDirectionalLightDiffuse(light, worldNormal);
        specular += CalcDirectionalLightSpecular(light, calcSpecular, CalcBlinnPhongSpecular(normalize(-directionalLights[i].direction.xyz), worldNormal, normalize(viewPos.xyz - input.worldPos), params.w));
    }
    for(int i = 0; i < params.z; i++) {
        diffuse += CalcSpotLightDiffuse(spotLights[i], worldNormal, input.worldPos);
        specular += CalcSpotLightSpecular(spotLights[i], input.worldPos, calcSpecular, CalcBlinnPhongSpecular(normalize(spotLights[i].position.xyz - input.worldPos), worldNormal, normalize(viewPos.xyz - input.worldPos), params.w));
    }

    float3 lighting = (calcAmbient.xyz + diffuse) * calcAlbedo.xyz + specular;
    return float4(lighting, calcAlbedo.w);
}
