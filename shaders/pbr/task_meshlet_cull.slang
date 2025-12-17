import modules.constants;
import modules.bindless;
import modules.common;
import modules.indirect_draw;
import modules.meshlet_cull;
import modules.visibility;

groupshared TaskPayload sharedPayload;

// Task Shader --------------------

[shader("task")]
[numthreads(TASK_GROUP_SIZE, 1, 1)]
func taskMain(
    uint drawID: SV_DrawIndex,
    uint3 groupID: SV_GroupID,
    uint3 groupThreadID: SV_GroupThreadID,
    uint3 dispatchThreadID: SV_DispatchThreadID)
{
    if (drawID >= indirectDraw::pc.batchSize)
        return;

    let instanceID = indirectDraw::pc.visibility.get()[indirectDraw::pc.batchFirstID + drawID];
    let instance = indirectDraw::getInstance(instanceID);
    let mesh = instance.mesh.get();

    bool meshletSurvives = false;
    if (dispatchThreadID.x < mesh.meshletCount)
    {
        let camera = indirectDraw::pc.camera.get();
        let meshlet = mesh.meshlet.get()[dispatchThreadID.x];
        let worldBounds = meshlet.bounds.transform(instance.modelMatrix);

        meshletSurvives = visibility::frustumVisible(worldBounds, camera.frustum);
    }

    uint numSurvive = WaveActiveCountBits(meshletSurvives);
    uint idxOffset = WavePrefixCountBits(meshletSurvives);

    if (meshletSurvives)
    {
        sharedPayload.meshletIDs[idxOffset] = uint8_t(groupThreadID.x);
    }

    if (groupThreadID.x == 0)
    {
        sharedPayload.groupMeshletOffset = groupID.x * TASK_GROUP_SIZE;
        sharedPayload.instanceID = instanceID;
        DispatchMesh(numSurvive, 1, 1, sharedPayload);
    }
}
