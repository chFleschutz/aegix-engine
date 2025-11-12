struct VSIn
{
    [vk::location(0)] float3 position;
};

struct VSOut
{
    float4 position: SV_Position;
    float3 localPos;
};

struct FSOut
{
    [vk::location(0)] float4 color;
};

struct Push
{
    float4x4 viewProjection;
};

[vk::push_constant] ConstantBuffer<Push> push;
[vk::binding(0)] SamplerCube skyboxMap;

// Vertex Shader ---------------------------

[shader("vertex")]
func vertexMain(VSIn input, out VSOut output)
{
    float4 clipPos = mul(push.viewProjection, float4(input.position, 1.0));
    output.position = clipPos.xyww; // Ensure maximum depth (z = 1.0)
    output.localPos = input.position;
}

// Fragment Shader ---------------------------

[shader("fragment")]
func fragmentMain(VSOut input, out FSOut output)
{
    output.color = float4(skyboxMap.Sample(input.localPos).rgb, 1.0);
}
