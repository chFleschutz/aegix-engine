#version 450

layout(push_constant, std140) uniform Push
{
	mat4 view;
	mat4 projection;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 outPosition;

void main()
{
	outPosition = inPosition;
	vec4 clipPos = push.projection * mat4(mat3(push.view)) * vec4(outPosition, 1.0);
	gl_Position = clipPos.xyww; // Ensure maximum depth (z = 1.0)
}