module indirectDraw;

import modules.bindless;
import modules.geometry;

namespace indirectDraw
{
    public struct Instance
    {
        public float3x4 transform;
        public float3 normalRow0;
        public bindless::Handle<UniformBuffer<geometry::Mesh>> mesh;
        public float3 normalRow1;
        public bindless::Handle material;
        public float3 normalRow2;
        public uint drawBatchID;

        public property float4x4 modelMatrix
        {
            get { return float4x4(transform, float4(0.0, 0.0, 0.0, 1.0)); }
        }

        public property float3x3 normalMatrix
        {
            get { return float3x3(normalRow0, normalRow1, normalRow2); }
        }
    }

    public struct PushConstant
    {
        public bindless::Handle<UniformBuffer<geometry::Global>> global;
        public bindless::Handle<StorageBuffer<Instance>> staticInstances;
        public bindless::Handle<StorageBuffer<Instance>> dynamicInstances;
        public bindless::Handle<StorageBuffer<uint>> visibility;
        public uint batchFirstID;
        public uint batchSize;
        public uint staticCount;
        public uint dynamicCount;
    }
    public [vk::push_constant] PushConstant pc;

    public func getInstance(uint index) -> Instance
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
}
