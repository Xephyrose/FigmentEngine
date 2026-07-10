#include "assets/shaders/includes/Lights.hlsl"

Texture2D g_albedo : register(t0, space2);
Texture2D g_ambient : register(t1, space2);
Texture2D g_specular : register(t2, space2);
Texture2D g_shadow_map : register(t3, space2);
SamplerState g_sampler0 : register(s0, space2);
SamplerState g_sampler1 : register(s1, space2);
SamplerState g_sampler2 : register(s2, space2);
SamplerComparisonState g_shadow_sampler : register(s3, space2);

cbuffer PushConstants : register(b0, space3)
{
    float3 viewPos;
    float  shininess;
    float4 colorAlbedo;
    bool   useAlbedoTexture;
    float3 colorAmbient;
    bool   useAmbientTexture;
    float3 colorSpecular;
    bool   useSpecularTexture;
    int    num_point_lights;
    int    num_dir_lights;
    int    num_spot_lights;
}

struct PSInput {
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
    float4 shadowCoord : TEXCOORD3;
};

StructuredBuffer<PointLight> pointLights : register(t4, space2);
StructuredBuffer<DirectionalLight> directionalLights : register(t5, space2);
StructuredBuffer<SpotLight> spotLights : register(t6, space2);

float4 main(PSInput input) : SV_TARGET {

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
        result += CalcPointLight(pointLights[i], input.worldNormal, input.worldPos, calcSpecular, CalcPhongSpecular(normalize(pointLights[i].position.xyz - input.worldPos), input.worldNormal, normalize(viewPos - input.worldPos), shininess));
    }
    for(int i = 0; i < num_dir_lights; i++) {
       DirectionalLight light = directionalLights[i];
       light.direction.w = CalcDirectionalLightShadows(light, g_shadow_map, g_shadow_sampler, input.shadowCoord, input.worldNormal, 1);
       result += CalcDirectionalLight(light, input.worldNormal, calcSpecular, CalcPhongSpecular(normalize(-directionalLights[i].direction.xyz), input.worldNormal, normalize(viewPos - input.worldPos), shininess));
    }
    for(int i = 0; i < num_spot_lights; i++) {
        result += CalcSpotLight(spotLights[i], input.worldNormal, input.worldPos, float3(1.0, 1.0, 1.0), CalcPhongSpecular(normalize(spotLights[i].position.xyz - input.worldPos), input.worldNormal, normalize(viewPos - input.worldPos), 64));
    }

    float3 lighting = (calcAmbient + result) * calcAlbedo;
    return float4(lighting, calcAlbedo.w);
}
