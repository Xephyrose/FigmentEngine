Texture2D g_albedo : register(t0, space2);
Texture2D g_ambient : register(t1, space2);
Texture2D g_specular : register(t2, space2);
SamplerState g_sampler0 : register(s0, space2);
SamplerState g_sampler1 : register(s1, space2);
SamplerState g_sampler2 : register(s2, space2);

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
};

struct PointLight {
    float4 color;
    float4 position; // 3 for pos, 1 for padding
};

struct DirectionalLight {
    float4 rgb;
    float4 direction; // 3 for dir, 1 for padding
};

//struct SpotLight {
//    float4 rgb;
//    float4 position; // 3 for pos, 1 for padding
//    float4 direction; // 3 for dir, 1 for padding
//};

StructuredBuffer<PointLight> pointLights : register(t3, space2);
StructuredBuffer<DirectionalLight> directionalLights : register(t4, space2);
//StructuredBuffer<SpotLight> spotLights : register(t5, space2);

float4 main(PSInput input) : SV_TARGET {

    float3 worldNormal = normalize(input.worldNormal);

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

    float3 lightColor = pointLights[0].rgb * pointLights[0].w;
    float3 lightPos = pointLights[1].xyz;

    // Ambient
    float3 ambient = lightColor * calcAmbient;

    // Diffuse
    float3 lightDir = normalize(lightPos - input.worldPos);
    float diff = max(dot(worldNormal, lightDir), 0.0);
    float3 diffuse = lightColor * diff * calcAlbedo.xyz;

    // Specular
    float3 viewDir = normalize(viewPos - input.worldPos);
    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(worldNormal, halfwayDir), 0.0), shininess * 4);
    float3 specular = lightColor * (spec * calcSpecular);

    float3 result = ambient + diffuse + specular;
    return float4(result, calcAlbedo.w);
}
