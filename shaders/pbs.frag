#version 450

layout(location = 0) in vec3 inPosWorld;
layout(location = 1) in vec3 inNormalWorld;
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

layout(push_constant) uniform Push 
{
    mat4 modelMatrix; 
    mat4 normalMatrix;
} push;

void main()
{
    outColor = vec4(material.albedo, 1.0);
}