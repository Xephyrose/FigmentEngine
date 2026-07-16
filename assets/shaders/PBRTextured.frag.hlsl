#include "assets/shaders/includes/Lights.hlsl"
#define PI 3.14159265359

Texture2D g_albedo : register(t0, space2);
Texture2D g_orm : register(t1, space2);
Texture2D g_normal_map : register(t2, space2);
//Texture2D g_shadow_map : register(t3, space2);
SamplerState g_sampler_albedo : register(s0, space2);
SamplerState g_sampler_orm : register(s1, space2);
SamplerState g_sampler_normal_map : register(s2, space2);
//SamplerComparisonState g_sampler_shadow : register(s3, space2);

cbuffer PushConstants : register(b0, space3)
{
    float4  viewPos;
    float4  colorAlbedo;
    uint4   texturesUsed; // albedo, orm
    float4  colorORM;
    float4 lightNums; // num_point_lights, num_dir_lights, num_spot_lights
}

struct PSInput {
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
    float3 worldTangent : TEXCOORD3;
    float3 worldBitangent : TEXCOORD4;
    float4 shadowCoord : TEXCOORD5;
};

StructuredBuffer<PointLight> pointLights : register(t3, space2);
StructuredBuffer<DirectionalLight> directionalLights : register(t4, space2);
StructuredBuffer<SpotLight> spotLights : register(t5, space2);

float DistributionGGX(float3 N, float3 H, float roughness){
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness){
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness){
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0){
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float4 main(PSInput input) : SV_TARGET {
    float4 calcAlbedo;
    if (texturesUsed.x == true) {
        float4 texColor = g_albedo.Sample(g_sampler_albedo, input.uv);
        calcAlbedo = texColor * colorAlbedo;
    }
    else {
        calcAlbedo = colorAlbedo;
    }

    float calcAO = colorORM.x;
    float calcRoughness = colorORM.y;
    float calcMetallic = colorORM.z;
    if (texturesUsed.y == true) {
        float3 texColor = g_orm.Sample(g_sampler_orm, input.uv).xyz;
        calcAO = texColor.x * colorORM.x;
        calcRoughness = texColor.y * colorORM.y;
        calcMetallic = texColor.z * colorORM.z;
    }


    float3 worldNormal = input.worldNormal;
    if (texturesUsed.z == true) {
        float3 sampledNormal = g_normal_map.Sample(g_sampler_normal_map, input.uv).rgb;
        float3 tangentNormal = sampledNormal * 2.0 - 1.0;

        float3 N = normalize(input.worldNormal);
        float3 T = normalize(input.worldTangent);
        float3 B = normalize(input.worldBitangent);

        T = normalize(T - dot(T, N) * N);
        B = cross(N, T);

        worldNormal = normalize(T * tangentNormal.x + B * tangentNormal.y + N * tangentNormal.z);
    }

    float3 V = normalize(viewPos.xyz - input.worldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, calcAlbedo.xyz, calcMetallic);

    // reflectance equation
    float3 Lo = float3(0, 0, 0);
    for(int i = 0; i < lightNums.x; ++i)
    {
        // calculate per-light radiance
        float3 L = normalize(pointLights[i].position.xyz - input.worldPos);
        float3 H = normalize(V + L);
        float distance = length(pointLights[i].position.xyz - input.worldPos);
        float attenuation = 1.0 / (distance * distance);
        float3 radiance = pointLights[i].color.xyz * pointLights[i].color.w * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(worldNormal, H, calcRoughness);
        float G   = GeometrySmith(worldNormal, V, L, calcRoughness);
        float3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        float3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(worldNormal, V), 0.0) * max(dot(worldNormal, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        float3 specular = numerator / denominator;

        // kS is equal to Fresnel
        float3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        float3 kD = float3(1, 1, 1) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - calcMetallic;

        // scale light by NdotL
        float NdotL = max(dot(worldNormal, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * calcAlbedo.xyz / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (note that the next IBL tutorial will replace this ambient lighting with environment lighting).
    float3 ambient = float3(0.03, 0.03, 0.03) * calcAlbedo.xyz * calcAO;

    float3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + float3(1, 1, 1));
    // gamma correct
    color = pow(color, float3(1/2.2, 1/2.2, 1/2.2));

    return float4(color, 1.0);
}
