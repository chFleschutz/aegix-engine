import modules.bindless;
import modules.constants;
import modules.geometry;
import modules.indirect_draw;

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
    bindless::Handle<StorageBuffer<indirectDraw::Instance>> staticInstances;
    bindless::Handle<StorageBuffer<indirectDraw::Instance>> dynamicInstances;
    bindless::Handle<StorageBuffer<DrawBatch>> drawBatches;
    bindless::Handle<RWStorageBuffer<uint>> visibility;
    bindless::Handle<RWStorageBuffer<DrawMeshTasksIndirectCommand>> indirectDrawCommands;
    bindless::Handle<RWStorageBuffer<uint>> indirectDrawCounts;
    uint staticCount;
    uint dynamicCount;
}

[vk_push_constant] PushConstant pc;

func getInstance(uint index) -> indirectDraw::Instance
{
    if (index < pc.staticCount)
        return pc.staticInstances.get()[index];
    return pc.dynamicInstances.get()[index - pc.staticCount];
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