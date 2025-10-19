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

struct Meshlet
{
    float3 center;
    float radius;
    int8_t coneAxis[3];
    int8_t coneCutoff;
    uint vertexOffset;
    uint primitiveOffset;
    uint8_t vertexCount;
    uint8_t primitiveCount;
};

struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
    float3 color;
};

struct MSOut
{
    float4 position : SV_Position;
    float3 worldPosition;
    float3 worldNormal;
    float2 uv;
    float4 color;
};

struct GBuffer
{
    [vk::location(0)] float4 position;
    [vk::location(1)] float4 normal;
    [vk::location(2)] float4 albedo;
    [vk::location(3)] float4 arm;
    [vk::location(4)] float4 emissive;
};

static const float PI = 3.14159265359;
static const float EPSILON = 1e-5;

[vk::push_constant] ConstantBuffer<Push> push;
[vk::binding(0, 0)] ConstantBuffer<Global> global;

[vk::binding(0, 1)] ConstantBuffer<Material> material;
[vk::binding(1, 1)] Sampler2D albedoMap;
[vk::binding(2, 1)] Sampler2D normalMap;
[vk::binding(3, 1)] Sampler2D metalRoughnessMap;
[vk::binding(4, 1)] Sampler2D aoMap;
[vk::binding(5, 1)] Sampler2D emissiveMap;

[vk::binding(0, 2)] StructuredBuffer<Meshlet> meshlets;
[vk::binding(0, 3)] StructuredBuffer<uint> vertexIndices;
[vk::binding(1, 3)] StructuredBuffer<uint8_t, ScalarDataLayout> primitives;
[vk::binding(2, 3)] StructuredBuffer<Vertex, ScalarDataLayout> vertices;

func calcTBN(MSOut input) -> float3x3
{
    float3 Q1 = ddx(input.worldPosition);
    float3 Q2 = ddy(input.worldPosition);
    float2 st1 = ddx(input.uv);
    float2 st2 = ddy(input.uv);

    float3 N = normalize(input.worldNormal);
    float3 T = normalize(Q1 * st2.y - Q2 * st1.y + EPSILON);
    float3 B = normalize(cross(N, T));
    return float3x3(T, B, N);
}

// Mesh Shader --------------------------------------------------

[shader("mesh")]
[numthreads(32, 1, 1)]
[outputtopology("triangle")]
func meshMain(
    uint3 groupID: SV_GroupID,
    uint3 threadID: SV_GroupThreadID,
    out vertices MSOut meshVertices[64],
    out indices uint3 meshPrimitives[126])
{
    Meshlet meshlet = meshlets[groupID.x];
    SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

    float4x4 viewProjection = mul(global.projection, global.view);

    // Emit vertices
    for (uint i = threadID.x; i < uint(meshlet.vertexCount); i += 32)
    {
        uint index = vertexIndices[meshlet.vertexOffset + i];
        let v = vertices[index];

        float4 worldPos = mul(push.model, float4(v.position, 1.0));
        meshVertices[i].position = mul(viewProjection, worldPos);
        meshVertices[i].worldPosition = worldPos.xyz;
        meshVertices[i].worldNormal = normalize(mul(float3x3(push.normal), v.normal));
        meshVertices[i].uv = v.uv;
    }

    // Emit primitives
    for (uint i = threadID.x; i < uint(meshlet.primitiveCount); i += 32)
    {
        uint offset = meshlet.primitiveOffset + i * 3;
        meshPrimitives[i] = uint3(primitives[offset + 0], primitives[offset + 1], primitives[offset + 2]);
    }
}

// Fragment Shader --------------------------------------------------

[shader("fragment")]
func fragmentMain(in MSOut input, out GBuffer output) 
{
    float3 albedo = albedoMap.Sample(input.uv).rgb * material.albedo;
    float3 normal = normalMap.Sample(input.uv).rgb * 2.0 - 1.0;
    float3 emissive = emissiveMap.Sample(input.uv).rgb * material.emissive;
    float2 metalicRoughness = metalRoughnessMap.Sample(input.uv).bg;
    float metallic = metalicRoughness.r * material.metallic;
    float roughness = metalicRoughness.g * material.roughness;
    float ao = aoMap.Sample(input.uv).r * material.ao;

    float3 N = normalize(length(normal) < 0.1 ? input.worldNormal : mul(normal, calcTBN(input)));

    output.position = float4(input.worldPosition, 1.0);
    output.normal = float4(N, 0.0);
    output.albedo = float4(albedo, 1.0);
    output.arm = float4(ao, roughness, metallic, 0.0);
    output.emissive = float4(emissive, 1.0);
}
