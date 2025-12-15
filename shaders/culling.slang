import modules.bindless;
import modules.constants;
import modules.geometry;

// TODO: Move this into a module
struct Instance
{
    float3x4 transform;
    float3 normalRow0;
    bindless::Handle<UniformBuffer<geometry::Mesh>> mesh;
    float3 normalRow1;
    bindless::Handle<Unused> material;
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

struct DrawBatch
{
    uint offset;
    uint count;
}

struct DrawMeshTasksIndirectCommand
{
    uint groupCountX;
    uint groupCountY;
    uint groupCountZ;
}

struct PushConstant
{
    bindless::Handle<StorageBuffer<Instance>> staticInstances;
    bindless::Handle<StorageBuffer<Instance>> dynamicInstances;
    bindless::Handle<StorageBuffer<DrawBatch>> drawBatches;
    bindless::Handle<RWStorageBuffer<uint>> visibility;
    bindless::Handle<RWStorageBuffer<DrawMeshTasksIndirectCommand>> indirectDrawCommands;
    bindless::Handle<RWStorageBuffer<uint>> indirectDrawCounts;
    uint staticCount;
    uint dynamicCount;
}

[vk_push_constant] PushConstant pc;

func getInstance(uint index) -> Instance
{
    if (index < pc.staticCount)
    {
        return pc.staticInstances.get()[index];
    }
    else
    {
        return pc.dynamicInstances.get()[index - pc.staticCount];
    }
}

[shader("compute")]
[numthreads(64, 1, 1)]
func main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= pc.staticCount + pc.dynamicCount)
        return;

    let instanceID = dispatchThreadID.x;
    let instance = getInstance(instanceID);

    // TODO: Do culling

    uint drawID;
    InterlockedAdd(pc.indirectDrawCounts.get()[instance.drawBatchID], 1, drawID);

    let drawBatch = pc.drawBatches.get()[instance.drawBatchID];
    pc.visibility.get()[drawBatch.offset + drawID] = instanceID;

    let mesh = instance.mesh.get();
    pc.indirectDrawCommands.get()[drawBatch.offset + drawID] = DrawMeshTasksIndirectCommand(1, 1, 1);
}