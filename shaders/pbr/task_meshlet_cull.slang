import modules.constants;
import modules.bindless;
import modules.common;
import modules.indirect_draw;

static const uint TASK_GROUP_SIZE = 1;

struct TaskPayload
{
    uint instanceID;
}

groupshared TaskPayload sharedPayload;

// Task Shader --------------------

[shader("task")]
[numthreads(TASK_GROUP_SIZE, 1, 1)]
func taskMain(uint3 threadID: SV_GroupThreadID, uint drawID: SV_DrawIndex)
{
    if (drawID >= indirectDraw::pc.batchSize)
        return;

    let instanceID = indirectDraw::pc.visibility.get()[indirectDraw::pc.batchFirstID + drawID];
    let instance = indirectDraw::getInstance(instanceID);
    let mesh = instance.mesh.get();

    GroupMemoryBarrierWithGroupSync();
    if (threadID.x == 0)
    {
        sharedPayload.instanceID = instanceID;
        DispatchMesh(mesh.meshletCount, 1, 1, sharedPayload);
    }

    // TODO: Meshlet culling
}
