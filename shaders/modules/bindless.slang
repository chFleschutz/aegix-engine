module bindless;

namespace bindless
{
    public static const uint SAMPLED_IMAGE = 0;
    public static const uint STORAGE_IMAGE = 1;
    public static const uint STORAGE_BUFFER = 2;
    public static const uint UNIFORM_BUFFER = 3;

    [vk::binding(SAMPLED_IMAGE)] Sampler2D sampledImages[];
    [vk::binding(STORAGE_IMAGE)] RWTexture2D storageImages[];

    /// @note This has to match the DescriptorHandle class on the C++ side
    /// @see src/aegix/graphics/bindless/DescriptorHandle.h
    public struct Handle
    {
        enum Type : uint
        {
            SampledImage = 0,
            StorageImage = 1,
            StorageBuffer = 2,
            UniformBuffer = 3,
        };

        static const uint INDEX_BITS = 24;
        static const uint VERSION_BITS = 6;
        static const uint TYPE_BITS = 2;

        static const uint INDEX_MASK = (1 << INDEX_BITS) - 1;
        static const uint VERSION_MASK = (1 << VERSION_BITS) - 1;
        static const uint TYPE_MASK = (1 << TYPE_BITS) - 1;

        // Packed as: | 2 bits type | 6 bits version | 24 bits index |
        uint handle;

        public property uint index { get { return handle & INDEX_MASK; } }
        public property uint type { get { return (handle >> INDEX_BITS) & TYPE_MASK; } }
        public property uint version { get { return (handle >> (INDEX_BITS + TYPE_BITS)) & VERSION_MASK; } }
        
        public func Sample(float2 uv) -> float4
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