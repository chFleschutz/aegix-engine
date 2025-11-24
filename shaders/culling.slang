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

    property float4x4 modelMatrix
    {
        get { return float4x4(transform, float4(0.0, 0.0, 0.0, 1.0)); }
    }

    property float3x3 normalMatrix
    {
        get { return float3x3(normalRow0, normalRow1, normalRow2); }
    }
};

struct IndirectMeshDrawArgs
{
    uint groupCountX;
    uint groupCountY;
    uint groupCountZ;
};

struct Push
{
    bindless::Handle<RWStorageBuffer<uint>> visibility;
    bindless::Handle<RWStorageBuffer<uint>> visibleCount;
    bindless::Handle<StorageBuffer<Instance>> instance;
    uint instanceCount;
}

[vk_push_constant] Push push;

[shader("compute")]
[numthreads(16, 1, 1)]
func main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= push.instanceCount)
        return;

    let instanceID = dispatchThreadID.x;
    let instance = push.instance.get()[instanceID];

    // TODO: Do culling

    // TODO: Get batch index
    uint batchID = 0;
    InterlockedAdd(push.visibleCount.get()[batchID], 1);

    // TODO: Calc offset in visibility buffer
    uint offset = 0;
    push.visibility.get()[offset] = instanceID;

    // TODO: Fill indirect draw buffer

}