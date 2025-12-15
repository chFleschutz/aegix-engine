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

struct Push
{
    bindless::Handle<StorageBuffer<Instance>> instance;
    bindless::Handle<StorageBuffer<DrawBatch>> drawBatch;
    bindless::Handle<RWStorageBuffer<uint>> visibility;
    bindless::Handle<RWStorageBuffer<uint>> visibleCount;
    uint instanceCount;
}

[vk_push_constant] Push push;

[shader("compute")]
[numthreads(64, 1, 1)]
func main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= push.instanceCount)
        return;

    let instanceID = dispatchThreadID.x;
    let instance = push.instance.get()[instanceID];

    // TODO: Do culling

    uint count;
    InterlockedAdd(push.visibleCount.get()[instance.drawBatchID], 1, count);

    let drawBatch = push.drawBatch.get()[instance.drawBatchID];
    push.visibility.get()[drawBatch.offset + count] = instanceID;

    // TODO: Fill indirect draw buffer
}