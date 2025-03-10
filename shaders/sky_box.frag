#version 450

//layout(set = 0, binding = 0) uniform samplerCube skyboxMap;

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec4 outColor;

void main()
{
//	outColor = texture(skyboxMap, inPosition);
//	outColor = vec4(0.0, 0.0, 1.0, 1.0);
	outColor = vec4(clamp(inPosition * 0.5 + 0.5, 0.0, 1.0), 1.0);
}