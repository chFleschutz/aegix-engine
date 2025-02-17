#version 450

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D albedoMap;
layout(set = 0, binding = 1) uniform sampler2D normalMap;

void main()
{
	vec3 albedo = texture(albedoMap, inUV).rgb;
	outColor = vec4(albedo, 1.0);
}
