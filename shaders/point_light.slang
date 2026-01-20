import "modules/constants";

static const float LIGHT_RADIUS = 0.1;
static const float2 OFFSETS[6] = float2[](
    float2(-1.0, -1.0),
    float2(1.0, -1.0),
    float2(-1.0, 1.0),
    float2(-1.0, 1.0),
    float2(1.0, -1.0),
    float2(1.0, 1.0)
);

struct VSOut
{
    float4 position : SV_Position;
    float2 fragOffset;
}

struct FSOut
{
    [vk::location(0)] float4 color;
}

struct PushConstants
{
    float4 worldPosition;
    float4 color;
    float radius;
}

struct GlobalUbo
{
    float4x4 view;
    float4x4 projection;
}

[vk::push_constant] PushConstants pc;
[vk::binding(0)] ConstantBuffer<GlobalUbo> ubo;

// Vertex shader --------------------

[shader("vertex")]
func vertexMain(uint vertexID: SV_VertexID, out VSOut output)
{
    output.fragOffset = OFFSETS[vertexID];
    float4 posOffset = float4(output.fragOffset, 0.0, 0.0) * pc.radius;
    float4 viewPosition = mul(ubo.view, pc.worldPosition);
    output.position = mul(ubo.projection, viewPosition + posOffset);
}

// Fragment shader ------------------

[shader("fragment")]
func fragmentMain(VSOut input, out FSOut output)
{
    float offsetDistance = dot(input.fragOffset, input.fragOffset);
    if (offsetDistance > 1.0)
        discard;

    float cosDistance = (cos(offsetDistance * constants::PI) + 1.0) * 0.5;
    output.color = float4(pc.color.xyz + cosDistance, cosDistance);
}
