#version 450

layout(location = 0) in vec2 fragOffset;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo 
{
    mat4 view;
    mat4 projection;
} ubo;

layout(push_constant) uniform Push 
{
    vec4 position;
    vec4 color;
    float radius;
} push;

const float M_PI = 3.1415926538;

void main()
{
    float offsetDistance = dot(fragOffset, fragOffset);
    if (offsetDistance > 1.0)
        discard;

    float cosDistance = 0.5 * (cos(offsetDistance * M_PI) + 1.0);
    outColor = vec4(push.color.xyz + cosDistance, cosDistance);
}