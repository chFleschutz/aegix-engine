#version 450

layout(set = 0, binding = 0) uniform samplerCube skyboxMap;

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(texture(skyboxMap, inPosition).rgb, 1.0);
}