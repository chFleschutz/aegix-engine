import modules.bindless;
import modules.constants;
import modules.geometry;
import modules.tbn;

static const uint TASK_THREADS = 1;
static const uint MESH_THREADS = 32;
static const uint MAX_VERTICES = 64;
static const uint MAX_PRIMITIVES = 126;

struct Material
{
    float3 albedo;
    float3 emissive;
    float metallic;
    float roughness;
    float ao;
    bindless::Handle<SampledImage2D> albedoMap;
    bindless::Handle<SampledImage2D> normalMap;
    bindless::Handle<SampledImage2D> metalRoughnessMap;
    bindless::Handle<SampledImage2D> aoMap;
    bindless::Handle<SampledImage2D> emissiveMap;
}

struct Instance
{
    float3x4 transform;
    float3 normalRow0;
    bindless::Handle<UniformBuffer<geometry::Mesh>> mesh;
    float3 normalRow1;
    bindless::Handle<UniformBuffer<Material>> material;
    float3 normalRow2;
    uint drawBatchID;

    property float4x4 modelMatrix
    {
        get { return float4x4(transform, float4(0.0, 0.0, 0.0, 1.0)); }
    }

    property float3x3 normalMatrix
    {
        get { return float3x3(normalRow0, normalRow1, normalRow2); }
    }
}

struct TaskPayload
{
    uint instanceID;
}

struct MSOut
{
    float4 position : SV_Position;
    float3 worldPosition;
    float3 worldNormal;
    float2 uv;
    nointerpolation uint instanceID;
}

public struct Push
{
    bindless::Handle<UniformBuffer<geometry::Global>> global;
    bindless::Handle<StorageBuffer<Instance>> instance;
    bindless::Handle<StorageBuffer<uint>> visibility;
    uint firstInstance;
    uint instanceCount;
}
[vk::push_constant] ConstantBuffer<Push> push;

groupshared TaskPayload taskPayload;

// Task Shader --------------------

[shader("task")]
[numthreads(TASK_THREADS, 1, 1)]
func taskMain(uint3 groupID: SV_GroupID, uint3 threadID: SV_GroupThreadID)
{
    if (groupID.x >= push.instanceCount)
        return;

    let instanceID = push.visibility.get()[push.firstInstance + groupID.x];
    let instance = push.instance.get()[instanceID];
    let mesh = instance.mesh.get();

    taskPayload.instanceID = instanceID;
    DispatchMesh(mesh.meshletCount, 1, 1, taskPayload);

    // TODO: Meshlet culling
}

// Mesh Shader --------------------

[shader("mesh")]
[numthreads(MESH_THREADS, 1, 1)]
[outputtopology("triangle")]
func meshMain(
    uint3 groupID: SV_GroupID, 
    uint3 threadID: SV_GroupThreadID,
    out vertices MSOut meshVertices[MAX_VERTICES], 
    out indices uint3 meshPrimitives[MAX_PRIMITIVES])
{
    let global = push.global.get();
    let instance = push.instance.get()[taskPayload.instanceID];
    let mesh = instance.mesh.get();
    let meshlet = mesh.meshlet.get()[groupID.x];

    SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

    float4x4 viewProjection = mul(global.projection, global.view);

    // Emit vertices
    for (uint i = threadID.x; i < uint(meshlet.vertexCount); i += MESH_THREADS)
    {
        uint index = mesh.meshletIndex.get()[meshlet.vertexOffset + i];
        let v = mesh.vertex.get()[index];

        float4 worldPos = mul(instance.modelMatrix, float4(v.position, 1.0));
        meshVertices[i].position = mul(viewProjection, worldPos);
        meshVertices[i].worldPosition = worldPos.xyz;
        meshVertices[i].worldNormal = normalize(mul(instance.normalMatrix, v.normal));
        meshVertices[i].uv = v.uv;
        meshVertices[i].instanceID = taskPayload.instanceID;
    }

    // Emit primitives
    for (uint i = threadID.x; i < uint(meshlet.primitiveCount); i += MESH_THREADS)
    {
        uint offset = meshlet.primitiveOffset + i * 3;
        meshPrimitives[i] = uint3(
            mesh.meshletPrimitive.get()[offset + 0], 
            mesh.meshletPrimitive.get()[offset + 1],
            mesh.meshletPrimitive.get()[offset + 2]);
    }
}


// Fragment Shader --------------------

[shader("fragment")]
func fragmentMain(MSOut input, out geometry::GBuffer output)
{
    let instance = push.instance.get()[input.instanceID];
    let mat = instance.material.get();

    float3 albedo = mat.albedoMap.get().Sample(input.uv).rgb * mat.albedo;
    float3 normal = mat.normalMap.get().Sample(input.uv).rgb * 2.0 - 1.0;
    float3 emissive = mat.emissiveMap.get().Sample(input.uv).rgb * mat.emissive;
    float2 metalicRoughness = mat.metalRoughnessMap.get().Sample(input.uv).bg;
    float metallic = metalicRoughness.r * mat.metallic;
    float roughness = metalicRoughness.g * mat.roughness;
    float ao = mat.aoMap.get().Sample(input.uv).r * mat.ao;

    let TBN = TBN::calcMatrix(input.worldPosition, input.worldNormal, input.uv);
    float3 N = normalize(length(normal) < 0.1 ? input.worldNormal : mul(normal, TBN));

    output.position = float4(input.worldPosition, 1.0);
    output.normal = float4(N, 0.0);
    output.albedo = float4(albedo, 1.0);
    output.arm = float4(ao, roughness, metallic, 0.0);
    output.emissive = float4(emissive, 1.0);
}
