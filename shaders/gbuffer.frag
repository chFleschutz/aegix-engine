#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inWorldNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outARM;       // Ambient, Roughness, Metallic
layout(location = 4) out vec4 outEmissive;

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
} global;

layout(set = 1, binding = 0, std140) uniform Material 
{
	vec3 albedo;
    vec3 emissive;
	float metallic;
	float roughness;
	float ambientOcclusion;
} material;

layout(set = 1, binding = 1) uniform sampler2D albedoMap;
layout(set = 1, binding = 2) uniform sampler2D normalMap;
layout(set = 1, binding = 3) uniform sampler2D metalRoughnessMap;
layout(set = 1, binding = 4) uniform sampler2D ambientOcclusionMap;
layout(set = 1, binding = 5) uniform sampler2D emissiveMap;

layout(push_constant) uniform Push 
{
    mat4 modelMatrix; 
    mat4 normalMatrix;
} push;

const float PI = 3.14159265359;

mat3 calcTBN();

void main()
{
    vec3 albedo = texture(albedoMap, inUV).rgb * material.albedo;
    vec3 normal = texture(normalMap, inUV).rgb * 2.0 - 1.0; 
    vec3 emissive = texture(emissiveMap, inUV).rgb * material.emissive;
    float metallic = texture(metalRoughnessMap, inUV).b * material.metallic;
    float roughness = texture(metalRoughnessMap, inUV).g * material.roughness;
    float ambientOcclusion = texture(ambientOcclusionMap, inUV).r * material.ambientOcclusion;

    vec3 N = normalize(length(normal) < 0.1 ? inWorldNormal : calcTBN() * normal);

    outPosition = vec4(inWorldPos, 1.0);
    outNormal = vec4(N, 0.0);
    outAlbedo = vec4(albedo, 1.0);
    outARM = vec4(ambientOcclusion, roughness, metallic, 0.0);
    outEmissive = vec4(emissive, 1.0);
}

mat3 calcTBN()
{
    vec3 Q1 = dFdx(inWorldPos);
    vec3 Q2 = dFdy(inWorldPos);
    vec2 st1 = dFdx(inUV);
    vec2 st2 = dFdy(inUV);

    vec3 N = normalize(inWorldNormal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));

    return mat3(T, B, N);
}
