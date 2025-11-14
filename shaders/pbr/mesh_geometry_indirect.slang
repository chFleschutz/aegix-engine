import modules.bindless;
import modules.constants;
import modules.geometry;
import modules.object;
import modules.tbn;

static const uint TASK_THREADS = 1;
static const uint MESH_THREADS = 32;
static const uint MAX_VERTICES = 64;
static const uint MAX_PRIMITIVES = 126;

public struct Push
{
    bindless::Handle global;
    bindless::Handle visibility;
    bindless::Handle instance;
};

struct Material
{
    float3 albedo;
    float3 emissive;
    float metallic;
    float roughness;
    float ao;
    bindless::Handle albedoMap;
    bindless::Handle normalMap;
    bindless::Handle metalRoughnessMap;
    bindless::Handle aoMap;
    bindless::Handle emissiveMap;
};

struct TaskPayload
{
    uint instanceID;
    uint meshletIDs[1024];
};

struct MSOut
{
    float4 position: SV_Position;
    float3 worldPosition;
    float3 worldNormal;
    float2 uv;
    nointerpolation uint instanceID;
};

[vk::push_constant] ConstantBuffer<Push> push;

[vk::binding(bindless::STORAGE_BUFFER)] StructuredBuffer<uint> visibilityBuffer[];
[vk::binding(bindless::STORAGE_BUFFER)] StructuredBuffer<object::Instance> instanceBuffer[];

[vk::binding(bindless::STORAGE_BUFFER)] StructuredBuffer<geometry::Meshlet> meshletBuffer[];
[vk::binding(bindless::STORAGE_BUFFER)] StructuredBuffer<uint> indexBuffer[];
[vk::binding(bindless::STORAGE_BUFFER)] StructuredBuffer<uint8_t, ScalarDataLayout> primitiveBuffer[];
[vk::binding(bindless::STORAGE_BUFFER)] StructuredBuffer<geometry::Vertex, ScalarDataLayout> vertexBuffer[];

[vk::binding(bindless::UNIFORM_BUFFER)] ConstantBuffer<geometry::Global, Std430DataLayout> globalBuffer[];
[vk::binding(bindless::UNIFORM_BUFFER)] ConstantBuffer<Material, Std430DataLayout> materialBuffer[];
[vk::binding(bindless::UNIFORM_BUFFER)] ConstantBuffer<geometry::Mesh, Std430DataLayout> meshBuffer[];

groupshared TaskPayload taskPayload;

// Task Shader --------------------

[shader("task")]
[numthreads(TASK_THREADS, 1, 1)]
func taskMain(uint3 groupID: SV_GroupID, uint3 threadID: SV_GroupThreadID)
{
    let instanceID = push.instance.get(visibilityBuffer)[groupID.x];
    let instance = push.instance.get(instanceBuffer)[instanceID];
    let mesh = instance.mesh.get(meshBuffer);
    let meshlets = mesh.meshlet.get(meshletBuffer);
    
    taskPayload.instanceID = instanceID;

    // Meshlet culling
    uint visibleCount = 0;
    for (uint i = 0; i < mesh.meshletCount; i++)
    {
        // TODO: Do meshlet culling here
        // Assume everything is visible for now

        taskPayload.meshletIDs[visibleCount++] = i;
    }

    if (visibleCount > 0)
    {
        DispatchMesh(visibleCount, 1, 1, taskPayload);
    }
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
    let global = push.global.get(globalBuffer);
    let instance = push.instance.get(instanceBuffer)[taskPayload.instanceID];
    let mesh = instance.mesh.get(meshBuffer);
    let meshletID = taskPayload.meshletIDs[groupID.x];
    let meshlet = mesh.meshlet.get(meshletBuffer)[meshletID];

    SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

    float4x4 viewProjection = mul(global.projection, global.view);

    // Emit vertices
    for (uint i = threadID.x; i < uint(meshlet.vertexCount); i += MESH_THREADS)
    {
        uint index = mesh.meshletIndex.get(indexBuffer)[meshlet.vertexOffset + i];
        let v = mesh.vertex.get(vertexBuffer)[index];

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
            mesh.meshletPrimitive.get(primitiveBuffer)[offset + 0], 
            mesh.meshletPrimitive.get(primitiveBuffer)[offset + 1],
            mesh.meshletPrimitive.get(primitiveBuffer)[offset + 2]);
    }
}

// Fragment Shader --------------------

[shader("fragment")]
func fragmentMain(MSOut input, out geometry::GBuffer output)
{
    let instance = push.instance.get(instanceBuffer)[input.instanceID];
    let mat = instance.material.get(materialBuffer);

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
