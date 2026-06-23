StructuredBuffer<float4> pointLights : register(t0, space2);

cbuffer PushConstants : register(b0, space3)
{
    float3 viewPos;
}

struct PSInput {
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
};

float4 main(PSInput input) : SV_TARGET {
    float3 lightColor = pointLights[0].rgb * pointLights[0].w;
    float3 lightPos = pointLights[1].xyz;

    // Ambient
    float ambientStrength = 0.1;
    float3 ambient = ambientStrength * lightColor;

    // Diffuse
    float3 norm = normalize(input.worldNormal);
    float3 lightDir = normalize(lightPos - input.worldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.5;
    float3 viewDir = normalize(viewPos - input.worldPos);
    float3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float3 specular = specularStrength * spec;

    float3 result = ambient + diffuse + specular;
    return float4(result, 1.0);
}
