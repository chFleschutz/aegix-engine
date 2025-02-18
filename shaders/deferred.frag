#version 450

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D positionMap;
layout(set = 0, binding = 1) uniform sampler2D normalMap;
layout(set = 0, binding = 2) uniform sampler2D albedoMap;
layout(set = 0, binding = 3) uniform sampler2D armMap;
layout(set = 0, binding = 4) uniform sampler2D emissiveMap;

void main()
{
	vec3 albedo = texture(albedoMap, inUV).rgb;
	outColor = vec4(albedo, 1.0);
}
