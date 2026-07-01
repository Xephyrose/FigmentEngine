#ifndef LIGHTS
#define LIGHTS

struct PointLight {
    float4 color; // xyz is color, w is padding
    float4 position; // xyz is position, w is padding
    float4 params; // x is constant, y is linear, z is quadratic, w is padding
};

struct DirectionalLight {
    float4 color; // xyz is color, w is intensity
    float4 direction; // 3 for dir, 1 for padding
};

struct SpotLight {
    float4 color; // xyz is color, w is padding
    float4 position; // xyz is position, w is cutoff
    float4 direction; // xyz is direction, w is outer cutoff
    float4 params; // x is constant, y is linear, z is quadratic, w is padding
};

float CalcPhongSpecular(float3 lightDir, float3 norm, float3 viewDir, float shininess) {
    float3 reflectDir = reflect(-lightDir, norm);
    return pow(max(dot(viewDir, reflectDir), 0.0), shininess);
}

float CalcBlinnPhongSpecular(float3 lightDir, float3 norm, float3 viewDir, float shininess) {
    float3 halfwayDir = normalize(lightDir + viewDir);
    return pow(max(dot(norm, halfwayDir), 0.0), shininess * 4);
}

float3 CalcPointLight(PointLight light, float3 normal, float3 fragPos, float3 calcAlbedo, float3 calcSpecular, float spec) {
    float3 lightColor = light.color.xyz;

    float3 lightDir = normalize(light.position.xyz - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.params.x + light.params.y * distance + light.params.z * (distance * distance));

    float3 diffuse = lightColor * diff * calcAlbedo.xyz;
    float3 specular = lightColor * (spec * calcSpecular);

    return (diffuse * attenuation) + (specular * attenuation);
}

float3 CalcDirectionalLight(DirectionalLight light, float3 normal, float3 calcAlbedo, float3 calcSpecular, float spec) {
    float3 lightColor = light.color.xyz * light.color.w;

    float3 lightDir = normalize(-light.direction.xyz);
    float diff = max(dot(normal, lightDir), 0.0);

    float3 diffuse = lightColor * diff * calcAlbedo;
    float3 specular = lightColor * spec * calcSpecular;
    return diffuse + specular;
}

float3 CalcSpotLight(SpotLight light, float3 normal, float3 fragPos, float3 calcAlbedo, float3 calcSpecular, float spec) {
    float3 lightColor = light.color.xyz;

    float3 lightDir = normalize(light.position.xyz - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.params.x + light.params.y * distance + light.params.z * (distance * distance));

    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon = light.position.w - light.direction.w;
    float intensity = clamp((theta - light.direction.w) / epsilon, 0.0, 1.0);

    float3 diffuse = lightColor * diff * calcAlbedo;
    float3 specular = lightColor * spec * calcSpecular;
    return (diffuse * attenuation * intensity) + (specular * attenuation * intensity);
}

#endif
