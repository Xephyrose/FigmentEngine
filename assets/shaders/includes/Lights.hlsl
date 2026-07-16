#ifndef LIGHTS
#define LIGHTS

#define PI 3.14159265359

struct PointLight {
    float4 color; // xyz is color, w is intensity
    float4 position; // xyz is position, w is padding
    float4 params; // x is constant, y is linear, z is quadratic, w is specular influence
};

struct DirectionalLight {
    float4 color; // xyz is color, w is intensity
    float4 direction; // 3 for direction, 1 for specular influence
};

struct SpotLight {
    float4 color; // xyz is color, w is intensity
    float4 position; // xyz is position, w is cutoff
    float4 direction; // xyz is direction, w is outer cutoff
    float4 params; // x is constant, y is linear, z is quadratic, w is specular influence
};

float CalcPhongSpecular(float3 lightDir, float3 norm, float3 viewDir, float shininess) {
    float3 reflectDir = reflect(-lightDir, norm);
    return pow(max(dot(viewDir, reflectDir), 0.0), shininess);
}

float CalcBlinnPhongSpecular(float3 lightDir, float3 norm, float3 viewDir, float shininess) {
    float3 halfwayDir = normalize(lightDir + viewDir);
    return pow(max(dot(norm, halfwayDir), 0.0), shininess * 4);
}

float3 CalcPointLightDiffuse(PointLight light, float3 normal, float3 fragPos) {
    float3 lightColor = light.color.xyz * light.color.w;

    float3 lightDir = normalize(light.position.xyz - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.params.x + light.params.y * distance + light.params.z * (distance * distance));

    float3 diffuse = lightColor * diff;

    return diffuse * attenuation * (light.params.w * 0.5 + 0.5);
}

float3 CalcPointLightSpecular(PointLight light, float3 fragPos, float3 calcSpecular, float spec) {
    float3 lightColor = light.color.xyz * light.color.w;

    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.params.x + light.params.y * distance + light.params.z * (distance * distance));

    float3 specular = lightColor * spec * calcSpecular;

    return specular * attenuation * light.params.w;
}

float3 CalcDirectionalLightDiffuse(DirectionalLight light, float3 normal) {
    float3 lightColor = light.color.xyz * light.color.w;

    float3 lightDir = normalize(-light.direction.xyz);
    float diff = max(dot(normal, lightDir), 0.0);

    float3 diffuse = lightColor * diff;
    return diffuse * light.direction.w;
}

float3 CalcDirectionalLightSpecular(DirectionalLight light, float3 calcSpecular, float spec) {
    float3 lightColor = light.color.xyz * light.color.w;

    float3 specular = lightColor * spec * calcSpecular;
    return specular * light.direction.w;
}

float3 CalcSpotLightDiffuse(SpotLight light, float3 normal, float3 fragPos) {
    float3 lightColor = light.color.xyz * light.color.w;

    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.params.x + light.params.y * distance + light.params.z * (distance * distance));

    float3 lightDir = normalize(light.position.xyz - fragPos);

    float theta = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon = light.position.w - light.direction.w;
    float intensity = clamp((theta - light.direction.w) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = lightColor * diff;
    return diffuse * attenuation * intensity * (light.params.w * 0.5 + 0.5);
}

float3 CalcSpotLightSpecular(SpotLight light, float3 fragPos, float3 calcSpecular, float spec) {
    float3 lightColor = light.color.xyz * light.color.w;
    float3 specular = lightColor * spec * calcSpecular;

    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.params.x + light.params.y * distance + light.params.z * (distance * distance));

    float3 lightDir = normalize(light.position.xyz - fragPos);

    float theta = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon = light.position.w - light.direction.w;
    float intensity = clamp((theta - light.direction.w) / epsilon, 0.0, 1.0);

    return specular * attenuation * intensity * light.params.w;
}

float CalcDirectionalLightShadows(DirectionalLight light, Texture2D shadowMap, SamplerComparisonState shadowSampler, float4 shadowCoord, float3 normal, int pcfDist) {
    float3 projCoords = shadowCoord.xyz / shadowCoord.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return 1.0;
    } else {
        float currentDepth = projCoords.z;
        float3 lightDir = normalize(-light.direction.xyz);
        float bias = max(0.001 * (1.0 - saturate(dot(normalize(normal), lightDir))), 0.0001);

        float shadow = 0.0;
        uint width, height;

        shadowMap.GetDimensions(width, height);
        float2 texelSize = 1.0 / float2(width, height);

        int div = 0;

        for(int x = -pcfDist; x <= pcfDist; ++x)
        {
            for(int y = -pcfDist; y <= pcfDist; ++y)
            {
                div += 1;
                float pcfDepth = shadowMap.SampleCmp(shadowSampler, projCoords.xy + float2(x, y) * texelSize, projCoords.z).r;
                shadow += currentDepth - bias > pcfDepth ? 0.0 : 1.0;
            }
        }
        shadow /= div;
        return shadow;
    }
}

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

#endif
