#ifndef LIGHTS
#define LIGHTS

// for all uses of color, they're vec4s with xyz as rgb, intensity is while
// positions and rotations are

struct PointLight {
    float4 color;
    float4 position; // 3 for pos, 1 for padding
};

struct DirectionalLight {
    float4 rgb;
    float4 direction; // 3 for dir, 1 for padding
};

struct SpotLight {
    float4 rgb;
    float4 position; // 3 for pos, 1 for padding
    float4 direction; // 3 for dir, 1 for padding
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
    float3 lightColor = light.color.xyz * light.color.w;

    float3 lightDir = normalize(light.position.xyz - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = lightColor * diff * calcAlbedo.xyz;
    float3 specular = lightColor * (spec * calcSpecular);

    return diffuse + specular;
}

float3 CalcDirectionalLight(DirectionalLight light, float3 normal, float3 calcAlbedo, float3 calcSpecular, float spec) {
    float3 lightColor = light.rgb.xyz * light.rgb.w;

    float3 lightDir = normalize(-light.direction.xyz);
    float diff = max(dot(normal, lightDir), 0.0);

    float3 diffuse = lightColor * diff * calcAlbedo;
    float3 specular = lightColor * spec * calcSpecular;
    return diffuse + specular;
}

#endif
