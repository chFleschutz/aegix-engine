struct AmbientLight
{
    float4 color;
};

struct DirectionalLight
{
    float4 direction;
    float4 color;
};

struct PointLight
{
    float4 position;
    float4 color;
};

struct Lighting
{
    float4 cameraPosition;
    AmbientLight ambientLight;
    DirectionalLight directionalLight;
    PointLight pointLights[128];
    int numPointLights;
    float ambientOcclusionFactor;
    int debugViewMode;
};

static const float PI = 3.14159265359;
static const float EPSILON = 0.0001;
static const float F_DIELECTRIC = 0.04;
static const float EMISSIVE_INTENSITY = 2.0;
static const float MAX_REFLECTION_LOD = 4.0;

[vk::binding(0, 0)] RWTexture2D<float4> sceneColorMap;
[vk::binding(1, 0)] Texture2D<float4> positionMap;
[vk::binding(2, 0)] Texture2D<float4> normalMap;
[vk::binding(3, 0)] Texture2D<float4> albedoMap;
[vk::binding(4, 0)] Texture2D<float4> armMap;
[vk::binding(5, 0)] Texture2D<float4> emissiveMap;
[vk::binding(6, 0)] Sampler2D ssaoMap;
[vk::binding(7, 0)] ConstantBuffer<Lighting> lighting;

[vk::binding(0, 1)] SamplerCube irradianceMap;
[vk::binding(1, 1)] SamplerCube prefilteredEnvMap;
[vk::binding(2, 1)] Sampler2D brdfLUTMap;

func sampleSSAO(float2 uv)->float
{
    float2 texelSize;
    ssaoMap.GetDimensions(texelSize.x, texelSize.y);
    texelSize = 1.0 / texelSize;

    float result = 0.0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            result += ssaoMap.SampleLevel(uv + offset, 0).r;
        }
    }
    return result / 16.0;
}

func lightAttenuation(float3 lightPos, float3 fragPos) -> float
{
    float distance = length(lightPos - fragPos);
    return 1.0 / max(distance * distance, EPSILON);
}

func specularReflection(float VdotH, float3 F0) -> float3
{
    return F0 + (float3(1.0) - F0) * pow(1.0 - VdotH, 5.0);
}

func specularReflection(float VdotH, float3 F0, float roughness) -> float3
{
    return F0 + (max(float3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

func schlickGGX(float NdotX, float alpha)->float
{
    // Schlick-Beckman GGX
    float k = alpha / 2.0;
    return NdotX / max(NdotX * (1.0 - k) + k, EPSILON);
}

func normalDistribution(float NdotH, float alpha)->float
{
    // Trowbridge-Reitz GGX
    float alpha2 = alpha * alpha;
    float denom = (NdotH * NdotH) * (alpha2 - 1.0) + 1.0;
    return alpha2 / max(PI * denom * denom, EPSILON);
}

func geometryOcclusion(float NdotV, float NdotL, float alpha)->float
{
    // Smith model with Schlick-GGX
    return schlickGGX(NdotV, alpha) * schlickGGX(NdotL, alpha);
}

func PBRLighting(float3 N, float3 V, float3 L, float3 albedo, float roughness, float metallic, float3 radiance, float3 F0) ->float3 
{
    float3 H = normalize(V + L);
    float alpha = roughness * roughness;
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    float D = normalDistribution(NdotH, alpha);
    float G = geometryOcclusion(NdotV, NdotL, alpha);
    float3 F = specularReflection(VdotH, F0);

    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metallic);

    float3 diffuse = kD * albedo / PI;
    float3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, EPSILON);
    return (diffuse + specular) * radiance * NdotL;
}

[shader("compute")]
[numthreads(16, 16, 1)]
func computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    int2 outputSize;
    sceneColorMap.GetDimensions(outputSize.x, outputSize.y);
    
    int2 pixelCoord = int2(dispatchThreadID.xy);
    float2 uv = float2(pixelCoord) / float2(outputSize);

    float3 position = positionMap[pixelCoord].xyz;
    float3 normal = normalMap[pixelCoord].xyz;
    float3 albedo = albedoMap[pixelCoord].xyz;
    float3 arm = armMap[pixelCoord].xyz;
    float3 emissive = emissiveMap[pixelCoord].xyz;
    float ssao = sampleSSAO(uv);

    float ao = arm.x * ssao;
    float roughness = arm.y;
    float metallic = arm.z;

    float3 N = normalize(normal);
    float3 V = normalize(lighting.cameraPosition.xyz - position);
    float3 R = reflect(-V, N);

    float NdotV = max(dot(N, V), 0.0);

    // Tint reflections for metallic surfaces
    float3 F0 = lerp(float3(F_DIELECTRIC), albedo, metallic);

    float3 Lo = emissive * EMISSIVE_INTENSITY;

    // Ambient Light
    {
        float3 F = specularReflection(NdotV, F0, roughness);
        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - metallic);

        float3 diffuse = albedo * irradianceMap.SampleLevel(N, 0).rgb;

        float3 prefilter = prefilteredEnvMap.SampleLevel(R, roughness * MAX_REFLECTION_LOD).rgb;
        float2 brdf = brdfLUTMap.SampleLevel(float2(NdotV, roughness), 0).rg;
        float3 specular = prefilter * (kS * brdf.x + brdf.y);

        float3 ambient = (kD * diffuse + specular) * ao;
        Lo += ambient * lighting.ambientLight.color.rgb * lighting.ambientLight.color.w;
    }
    // Directional Light
    {
        float3 L = normalize(lighting.directionalLight.direction.xyz);
        float3 radiance = lighting.directionalLight.color.rgb * lighting.directionalLight.color.w;
        Lo += PBRLighting(N, V, L, albedo, roughness, metallic, radiance, F0);
    }
    // Point Lights
    for (int i = 0; i < lighting.numPointLights; i++)
    {
        PointLight light = lighting.pointLights[i];
        float3 L = normalize(light.position.xyz - position);

        float attenuation = lightAttenuation(light.position.xyz, position);
        float3 radiance = light.color.rgb * light.color.w * attenuation;
        Lo += PBRLighting(N, V, L, albedo, roughness, metallic, radiance, F0);
    }

    Lo = max(Lo, 0.0);
    sceneColorMap[pixelCoord] = float4(Lo, 1.0);

    // Debug Views
    if (lighting.debugViewMode > 0)
    {
        switch (lighting.debugViewMode)
        {
        case 1:
            sceneColorMap[pixelCoord] = float4(albedo, 1.0);
            break;
        case 2:
            sceneColorMap[pixelCoord] = float4(float3(ao), 1.0);
            break;
        case 3:
            sceneColorMap[pixelCoord] = float4(float3(roughness), 1.0);
            break;
        case 4:
            sceneColorMap[pixelCoord] = float4(float3(metallic), 1.0);
            break;
        case 5:
            sceneColorMap[pixelCoord] = float4(emissive, 1.0);
            break;
        }
    }
}
