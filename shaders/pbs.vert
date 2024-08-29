#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outWorldPosition;
layout(location = 1) out vec3 outWorldNormal;
layout(location = 2) out vec2 outUV;

struct PointLight
{
    vec4 position;
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0, std140) uniform Global 
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
    vec3 emissiveFactor;
	float metallic;
	float roughness;
	float ambientOcclusion;
} material;

layout(push_constant, std140) uniform Push 
{
    mat4 modelMatrix; 
    mat4 normalMatrix;
} push;

void main()
{
    vec4 worldPosition = push.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = global.projection * global.view * worldPosition;

    outWorldPosition = worldPosition.xyz;
    outWorldNormal = normalize(mat3(push.normalMatrix) * inNormal);
    outUV = inUV;
}