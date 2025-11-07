module bindless;

namespace bindless
{
    public static const uint SAMPLED_IMAGE = 0;
    public static const uint STORAGE_IMAGE = 1;
    public static const uint STORAGE_BUFFER = 2;
    public static const uint UNIFORM_BUFFER = 3;

    [vk::binding(SAMPLED_IMAGE)] Sampler2D sampledImages[];
    [vk::binding(STORAGE_IMAGE)] RWTexture2D storageImages[];

    public struct Handle
    {
        public uint index;
        uint8_t version;
        uint8_t _;
        uint8_t type;
        uint8_t access;
        
        public func Sample(float2 uv)->float4
        {
            return sampledImages[index].Sample(uv);
        }

        public func Load(int2 coord) -> float4
        {
            return storageImages[index].Load(coord);
        }

        public func Store(int2 coord, float4 value)
        {
            storageImages[index].Store(coord, value);
        }

        public func get<T, L>(ConstantBuffer<T, L> buffers[]) -> ConstantBuffer<T, L> where L : IBufferDataLayout
        {
            return buffers[index];
        }

        public func get<T, L>(StructuredBuffer<T, L> buffers[]) -> StructuredBuffer<T, L> where L : IBufferDataLayout 
        {
            return buffers[index];
        }

        public func get<T, L>(RWStructuredBuffer<T, L> buffers[]) -> RWStructuredBuffer<T, L> where L : IBufferDataLayout 
        {
            return buffers[index];
        }
    };
}