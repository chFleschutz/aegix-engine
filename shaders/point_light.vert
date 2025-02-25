#version 450

const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0)
);

layout(location = 0) out vec2 fragOffset;

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

const float LIGHT_RADIUS = 0.1;

void main()
{
    fragOffset = OFFSETS[gl_VertexIndex];

    vec4 lightCameraSpace = ubo.view * push.position;
    vec4 positionCameraSpace = lightCameraSpace + vec4(fragOffset, 0.0, 0.0) * push.radius;
    gl_Position = ubo.projection * positionCameraSpace;
}