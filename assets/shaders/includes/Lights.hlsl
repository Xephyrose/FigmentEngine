#ifndef LIGHTS
#define LIGHTS

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

float3 CalcPointLight(PointLight light, float3 normal, float3 fragPos, float3 calcSpecular, float spec) {
    float3 lightColor = light.color.xyz * light.color.w;

    float3 lightDir = normalize(light.position.xyz - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.params.x + light.params.y * distance + light.params.z * (distance * distance));

    float3 diffuse = lightColor * diff;
    float3 specular = lightColor * (spec * calcSpecular);

    return (diffuse * attenuation * (light.params.w * 0.5 + 0.5)) + (specular * attenuation * light.params.w);
}

float3 CalcDirectionalLight(DirectionalLight light, float3 normal, float3 calcSpecular, float spec) {
    float3 lightColor = light.color.xyz * light.color.w;

    float3 lightDir = normalize(-light.direction.xyz);
    float diff = max(dot(normal, lightDir), 0.0);

    float3 diffuse = lightColor * diff;
    float3 specular = lightColor * spec * calcSpecular;
    return (diffuse + specular) * light.direction.w;
}

float3 CalcSpotLight(SpotLight light, float3 normal, float3 fragPos, float3 calcSpecular, float spec) {
    float3 lightColor = light.color.xyz * light.color.w;

    float3 lightDir = normalize(light.position.xyz - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.params.x + light.params.y * distance + light.params.z * (distance * distance));

    float theta = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon = light.position.w - light.direction.w;
    float intensity = clamp((theta - light.direction.w) / epsilon, 0.0, 1.0);

    float3 diffuse = lightColor * diff;
    float3 specular = lightColor * spec * calcSpecular;
    return (diffuse * attenuation * intensity * (light.params.w * 0.5 + 0.5)) + (specular * attenuation * intensity * light.params.w);
}

float CalcDirectionalLightShadows(DirectionalLight light, Texture2D shadowMap, SamplerComparisonState shadowSampler, float4 shadowCoord, float3 normal, int pcfDist) {
    float3 projCoords = shadowCoord.xyz / shadowCoord.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return 0.0;
    } else {
        float currentDepth = projCoords.z;
        float3 lightDir = normalize(-light.direction.xyz);
        float bias = max(0.0001 * (1.0 - saturate(dot(normalize(normal), lightDir))), 0.00001);

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

#endif
