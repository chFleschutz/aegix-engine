import "../modules/geometry";
import "../modules/constants";
import "../modules/tbn";

static const uint SAMPLED_IMAGE_BINDING = 1;
static const uint STORAGE_IMAGE_BINDING = 3;
static const uint STORAGE_BUFFER_BINDING = 6;
static const uint UNIFORM_BUFFER_BINDING = 8;

[vk::binding(SAMPLED_IMAGE_BINDING)] Sampler2D sampledImages[];
[vk::binding(STORAGE_IMAGE_BINDING)] RWTexture2D storageImages[];

[vk::binding(STORAGE_BUFFER_BINDING)] StructuredBuffer<geometry::Meshlet> meshlets[];
[vk::binding(STORAGE_BUFFER_BINDING)] StructuredBuffer<uint> vertexIndices[];
[vk::binding(STORAGE_BUFFER_BINDING)] StructuredBuffer<uint8_t, ScalarDataLayout> primitives[];
[vk::binding(STORAGE_BUFFER_BINDING)] StructuredBuffer<geometry::Vertex, ScalarDataLayout> vertices[];

[vk::binding(UNIFORM_BUFFER_BINDING)] ConstantBuffer<Global, Std430DataLayout> global[];
[vk::binding(UNIFORM_BUFFER_BINDING)] ConstantBuffer<Material, Std430DataLayout> material[];
[vk::binding(UNIFORM_BUFFER_BINDING)] ConstantBuffer<Mesh, Std430DataLayout> mesh[];

struct BindlessHandle
{
    uint index;
    uint8_t version;
    uint8_t _;
    uint8_t type;
    uint8_t access;

    func Sample(float2 uv) -> float4
    {
        return sampledImages[index].Sample(uv);
    }
};

struct MSOut
{
    float4 position : SV_Position;
    float3 worldPosition;
    float3 worldNormal;
    float2 uv;
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
    BindlessHandle albedoMap;
    BindlessHandle normalMap;
    BindlessHandle metalRoughnessMap;
    BindlessHandle aoMap;
    BindlessHandle emissiveMap;
};

struct Mesh
{
    BindlessHandle vertexBuffer;
    BindlessHandle meshletBuffer;
    BindlessHandle meshletIndexBuffer;
    BindlessHandle meshletPrimitiveBuffer;
};

struct Push
{
    float4x4 model;
    float4x4 normal;
    BindlessHandle global;
    BindlessHandle mesh;
    BindlessHandle material;
};

[vk::push_constant] ConstantBuffer<Push> push;

static const uint NUM_THREADS = 32;
static const uint MAX_VERTICES = 64;
static const uint MAX_PRIMITIVES = 126;

[shader("mesh")]
[numthreads(NUM_THREADS, 1, 1)]
[outputtopology("triangle")]
func meshMain(
    uint3 groupID: SV_GroupID,
    uint3 threadID: SV_GroupThreadID,
    out vertices MSOut meshVertices[MAX_VERTICES],
    out indices uint3 meshPrimitives[MAX_PRIMITIVES])
{
    Global global = global[push.global.index];
    Mesh mesh = mesh[push.mesh.index];
    geometry::Meshlet meshlet = meshlets[mesh.meshletBuffer.index][groupID.x];

    SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

    float4x4 viewProjection = mul(global.projection, global.view);

    // Emit vertices
    for (uint i = threadID.x; i < uint(meshlet.vertexCount); i += NUM_THREADS)
    {
        uint index = vertexIndices[mesh.meshletIndexBuffer.index][meshlet.vertexOffset + i];
        let v = vertices[mesh.vertexBuffer.index][index];

        float4 worldPos = mul(push.model, float4(v.position, 1.0));
        meshVertices[i].position = mul(viewProjection, worldPos);
        meshVertices[i].worldPosition = worldPos.xyz;
        meshVertices[i].worldNormal = normalize(mul(float3x3(push.normal), v.normal));
        meshVertices[i].uv = v.uv;
    }

    // Emit primitives
    for (uint i = threadID.x; i < uint(meshlet.primitiveCount); i += NUM_THREADS)
    {
        uint offset = meshlet.primitiveOffset + i * 3;
        meshPrimitives[i] = uint3(
            primitives[mesh.meshletPrimitiveBuffer.index][offset + 0], 
            primitives[mesh.meshletPrimitiveBuffer.index][offset + 1],
            primitives[mesh.meshletPrimitiveBuffer.index][offset + 2]);
    }
}

[shader("fragment")]
func fragmentMain(MSOut input, out geometry::GBuffer output)
{
    let mat = material[push.material.index];

    float3 albedo = mat.albedoMap.Sample(input.uv).rgb * mat.albedo;
    float3 normal = mat.normalMap.Sample(input.uv).rgb * 2.0 - 1.0;
    float3 emissive = mat.emissiveMap.Sample(input.uv).rgb * mat.emissive;
    float2 metalicRoughness = mat.metalRoughnessMap.Sample(input.uv).bg;
    float metallic = metalicRoughness.r * mat.metallic;
    float roughness = metalicRoughness.g * mat.roughness;
    float ao = mat.aoMap.Sample(input.uv).r * mat.ao;

    let TBN = TBN::calcMatrix(input.worldPosition, input.worldNormal, input.uv);
    float3 N = normalize(length(normal) < 0.1 ? input.worldNormal : mul(normal, TBN));

    output.position = float4(input.worldPosition, 1.0);
    output.normal = float4(N, 0.0);
    output.albedo = float4(albedo, 1.0);
    output.arm = float4(ao, roughness, metallic, 0.0);
    output.emissive = float4(emissive, 1.0);
}
