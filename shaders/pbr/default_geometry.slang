#language slang 2026

struct Push
{
    float4x4 model;
    float4x4 normal;
};

struct Global
{
    float4x4 projection;
    float4x4 view;
    float4x4 inverseView;
};

struct Material
{
    float3 albedo;
    float3 emissive;
    float metallic;
    float roughness;
    float ao;
};

struct Vertex
{
    [vk::location(0)] float3 position;
    [vk::location(1)] float3 normal;
    [vk::location(2)] float2 uv;
    [vk::location(3)] float3 color;
};

struct VSOut
{
    float4 position : SV_Position;
    [vk::location(0)] float3 worldPosition;
    [vk::location(1)] float3 worldNormal;
    [vk::location(2)] float2 uv;
};

struct GBuffer
{
    [vk::location(0)] float4 position;
    [vk::location(1)] float4 normal;
    [vk::location(2)] float4 albedo;
    [vk::location(3)] float4 arm;
    [vk::location(4)] float4 emissive;
};

[vk::push_constant] ConstantBuffer<Push> push;
[vk::binding(0, 0)] ConstantBuffer<Global> global;
[vk::binding(0, 1)] ConstantBuffer<Material> material;
[vk::binding(1, 1)] Sampler2D albedoMap;
[vk::binding(2, 1)] Sampler2D normalMap;
[vk::binding(3, 1)] Sampler2D metalRoughnessMap;
[vk::binding(4, 1)] Sampler2D aoMap;
[vk::binding(5, 1)] Sampler2D emissiveMap;

static const float PI = 3.14159265359;
static const float EPSILON = 1e-6;

func calcTBN(float3 worldPos, float3 worldNormal, float2 uv) -> float3x3
{
    float3 Q1 = ddx(worldPos);
    float3 Q2 = ddy(worldPos);
    float2 st1 = ddx(uv);
    float2 st2 = ddy(uv);

    float3 N = normalize(worldNormal);
    float3 T = normalize(Q1 * st2.y - Q2 * st1.y + EPSILON);
    float3 B = normalize(cross(N, T));
    return float3x3(T, B, N);
}

[shader("vertex")]
func vertexMain(in Vertex input, out VSOut output)
{
    float4 worldPosition = mul(push.model, float4(input.position, 1.0));

    output.position = mul(global.projection, mul(global.view, worldPosition));
    output.worldPosition = worldPosition.xyz;
    output.worldNormal = normalize(mul((float3x3)push.normal, input.normal));
    output.uv = input.uv;
}

[shader("fragment")]
func fragmentMain(in VSOut input, out GBuffer output)
{
    float3 albedo = albedoMap.Sample(input.uv).rgb * material.albedo;
    float3 normal = normalMap.Sample(input.uv).rgb * 2.0 - 1.0;
    float3 emissive = emissiveMap.Sample(input.uv).rgb * material.emissive;
    float2 metalicRoughness = metalRoughnessMap.Sample(input.uv).bg;
    float metallic = metalicRoughness.r * material.metallic;
    float roughness = metalicRoughness.g * material.roughness;
    float ao = aoMap.Sample(input.uv).r * material.ao;

    let TBN = calcTBN(input.worldPosition, input.worldNormal, input.uv);
    float3 N = normalize(length(normal) < 0.1 ? input.worldNormal : mul(normal, TBN));

    output.position = float4(input.worldPosition, 1.0);
    output.normal = float4(N, 0.0);
    output.albedo = float4(albedo, 1.0);
    output.arm = float4(ao, roughness, metallic, 0.0);
    output.emissive = float4(emissive, 1.0);
}
