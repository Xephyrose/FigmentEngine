Texture2D g_albedo : register(t0, space2);
Texture2D g_ambient : register(t1, space2);
Texture2D g_specular : register(t2, space2);
Texture2D g_normal_map : register(t3, space2);
SamplerState g_sampler0 : register(s0, space2);
SamplerState g_sampler1 : register(s1, space2);
SamplerState g_sampler2 : register(s2, space2);
SamplerState g_sampler3 : register(s3, space2);

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

StructuredBuffer<PointLight> pointLights : register(t4, space2);
StructuredBuffer<DirectionalLight> directionalLights : register(t5, space2);
//StructuredBuffer<SpotLight> spotLights : register(t6, space2);

float3 CalcPointLight(PointLight light, float3 normal, float3 fragPos, float3 viewDir, float3 calcAlbedo, float3 calcSpecular)
{
    float3 lightColor = light.color.xyz * light.color.w;

    // Diffuse
    float3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = lightColor * diff * calcAlbedo.xyz;

    // Specular
    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess * 4); // *4 because phong uses 4x as much as blinn-phong, so 4x here makes it comparable to phong
    float3 specular = lightColor * (spec * calcSpecular);

    return diffuse + specular;
}

float3 CalcDirectionalLight(DirectionalLight light, float3 normal, float3 fragPos, float3 viewDir, float3 calcAlbedo, float3 calcSpecular)
{
    float3 lightColor = light.rgb.xyz * light.rgb.w;

    float3 lightDir = normalize(-light.direction.xyz);
    float diff = max(dot(normal, lightDir), 0.0);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess * 4);

    float3 diffuse = lightColor * diff * calcAlbedo;
    float3 specular = lightColor * spec * calcSpecular;
    return diffuse + specular;
}

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
        result += CalcPointLight(pointLights[i], worldNormal, input.worldPos, normalize(viewPos - input.worldPos), calcAlbedo.xyz, calcSpecular);
    }
    for(int i = 0; i < num_dir_lights; i++) {
        result += CalcDirectionalLight(directionalLights[i], worldNormal, input.worldPos, normalize(viewPos - input.worldPos), calcAlbedo.xyz, calcSpecular);
    }
    return float4(result + calcAmbient, calcAlbedo.w);
}
