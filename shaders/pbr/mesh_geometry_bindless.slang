import "../modules/geometry";
import "../modules/constants";
import "../modules/tbn";

export func getDescriptorFromHandle<T>(DescriptorHandle<T> handle) -> T where T : IOpaqueDescriptor
{
    return defaultGetDescriptorFromHandle(handle, BindlessDescriptorOptions::None);
}

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
    DescriptorHandle<Sampler2D> albedoMap;
    DescriptorHandle<Sampler2D> normalMap;
    DescriptorHandle<Sampler2D> metalRoughnessMap;
    DescriptorHandle<Sampler2D> aoMap;
    DescriptorHandle<Sampler2D> emissiveMap;
};

struct MeshletMesh
{
    DescriptorHandle<StructuredBuffer<geometry::Meshlet>> meshlets;
    DescriptorHandle<StructuredBuffer<uint>> vertexIndices;
    DescriptorHandle<StructuredBuffer<uint8_t, ScalarDataLayout>> primitives;
    DescriptorHandle<StructuredBuffer<geometry::Vertex, ScalarDataLayout>> vertices;
};

struct Object
{
    float4x4 model;
    float4x4 normal;
    DescriptorHandle<StructuredBuffer<MeshletMesh>> mesh;
    DescriptorHandle<StructuredBuffer<Material>> material;
};

struct Push
{
    DescriptorHandle<StructuredBuffer<Global>> global;
    DescriptorHandle<StructuredBuffer<Object>> object;
    uint objectIndex;
    uint frameIndex;
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
    var global = push.global[push.frameIndex];
    var obj = push.object[push.objectIndex];
    var mesh = obj.mesh[0];
    var meshlet = mesh.meshlets[groupID.x];
    SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

    float4x4 viewProjection = mul(global.projection, global.view);

    // Emit vertices
    for (uint i = threadID.x; i < uint(meshlet.vertexCount); i += NUM_THREADS)
    {
        uint index = mesh.vertexIndices[meshlet.vertexOffset + i];
        let v = mesh.vertices[index];

        float4 worldPos = mul(obj.model, float4(v.position, 1.0));
        meshVertices[i].position = mul(viewProjection, worldPos);
        meshVertices[i].worldPosition = worldPos.xyz;
        meshVertices[i].worldNormal = normalize(mul(float3x3(obj.normal), v.normal));
        meshVertices[i].uv = v.uv;
    }

    // Emit primitives
    for (uint i = threadID.x; i < uint(meshlet.primitiveCount); i += NUM_THREADS)
    {
        uint offset = meshlet.primitiveOffset + i * 3;
        meshPrimitives[i] = uint3(mesh.primitives[offset + 0], mesh.primitives[offset + 1], mesh.primitives[offset + 2]);
    }
}

[shader("fragment")]
func fragmentMain(MSOut input, out geometry::GBuffer output)
{
    let obj = push.object[0];
    let mat = obj.material[0];

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
