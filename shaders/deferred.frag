#version 450

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D positionMap;
layout(set = 0, binding = 1) uniform sampler2D normalMap;
layout(set = 0, binding = 2) uniform sampler2D albedoMap;
layout(set = 0, binding = 3) uniform sampler2D armMap;
layout(set = 0, binding = 4) uniform sampler2D emissiveMap;

struct AmbientLight
{
	vec4 color;
};

struct DirectionalLight
{
	vec4 direction;
	vec4 color;
};

struct PointLight
{
	vec4 position;
	vec4 color;
};

layout(set = 0, binding = 5) uniform Lighting
{
	vec4 cameraPosition;
	AmbientLight ambientLight;
	DirectionalLight directionalLight;
	PointLight pointLights[10];
	int numPointLights;
} lighting;

const float PI = 3.14159265359;

float lightAttenuation(vec3 lightPos, vec3 fragPos);
float NormalDistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);

void main()
{
	vec3 position = texture(positionMap, inUV).rgb;
    vec3 normal = texture(normalMap, inUV).rgb;
	vec3 albedo = texture(albedoMap, inUV).rgb;
	vec3 arm = texture(armMap, inUV).rgb;
	vec3 emissive = texture(emissiveMap, inUV).rgb;

    float ambientOcclusion = arm.r;
    float roughness = arm.g;
    float metallic = arm.b;

	vec3 N = normalize(normal);
	vec3 V = normalize(lighting.cameraPosition.xyz - position);
	
    // Tint reflection for metals
    vec3 F0 = vec3(0.04); // default for dielectric materials
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < lighting.numPointLights; i++)
    {
        PointLight pointLight = lighting.pointLights[i];
        vec3 L = normalize(pointLight.position.xyz - position);
        vec3 H = normalize(V + L);

        // Light radiance
        float attenuation = lightAttenuation(pointLight.position.xyz, position);
        vec3 radiance = pointLight.color.rgb * pointLight.color.w * attenuation;

        // Cook-Torrance BRDF
        float D = NormalDistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metallic);

        vec3 DFG = D * G * F ;
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        vec3 specular = DFG / (4.0 * NdotV * NdotL + 0.0001);

        // Add light contribution
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Ambient light
    Lo += vec3(0.03) * albedo * ambientOcclusion;

    

    // Emissive material
    Lo += emissive;

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
