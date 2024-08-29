#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inWorldNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec4 outColor;

struct PointLight
{
    vec4 position;
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform Global 
{
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} global;

layout(set = 1, binding = 0, std140) uniform Material 
{
	vec3 albedo;
	float metallic;
	float roughness;
	float ambientOcclusion;
} material;

layout(set = 1, binding = 1) uniform sampler2D albedoMap;
layout(set = 1, binding = 2) uniform sampler2D normalMap;
layout(set = 1, binding = 3) uniform sampler2D metalRoughnessMap;
layout(set = 1, binding = 4) uniform sampler2D ambientOcclusionMap;

layout(push_constant) uniform Push 
{
    mat4 modelMatrix; 
    mat4 normalMatrix;
} push;

const float PI = 3.14159265359;

float lightAttenuation(vec3 lightPos, vec3 fragPos);
float NormalDistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);

void main()
{
    vec3 cameraPosition = vec3(global.inverseView[3]);
    vec3 N = normalize(inWorldNormal);
    vec3 V = normalize(cameraPosition - inWorldPos);

    // Tint reflection for metals
    vec3 F0 = vec3(0.04); // default for dielectric materials
    F0 = mix(F0, material.albedo, material.metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < global.numLights; i++)
    {
        PointLight light = global.pointLights[i];
        vec3 L = normalize(light.position.xyz - inWorldPos);
        vec3 H = normalize(V + L);

        // Light radiance
        float attenuation = lightAttenuation(light.position.xyz, inWorldPos);
        vec3 radiance = light.color.rgb * light.color.w * attenuation;

        // Cook-Torrance BRDF
        float D = NormalDistributionGGX(N, H, material.roughness);
        float G = GeometrySmith(N, V, L, material.roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - material.metallic);

        vec3 DFG = D * G * F ;
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        vec3 specular = DFG / (4.0 * NdotV * NdotL + 0.0001);

        // Add light contribution
        Lo += (kD * material.albedo / PI + specular) * radiance * NdotL;
    }

    // Ambient light
    Lo += vec3(0.03) * material.albedo * material.ambientOcclusion;

    outColor = vec4(Lo, 1.0);
}

float lightAttenuation(vec3 lightPos, vec3 fragPos)
{
	float d = length(lightPos - fragPos);
    return 1.0 / (d * d);
}

float NormalDistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denum = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denum * denum);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float denum = NdotV * (1.0 - k) + k;
    return NdotV / denum;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
