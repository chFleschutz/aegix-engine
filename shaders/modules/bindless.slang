module bindless;

// Bindless resource handle system for indexing resources via global descriptor arrays
//
// This works similar to slangs built-in DescriptorHandle, but allows for resources to be
// grouped by type at different binding slots (Required for Vulkan).
// Also each handle only uses 32 bits (instead of slangs 64 bits) and packs additional
// information for debugging like type and version.
//
// For type safety each handle stores the expected resource type as a generic parameter.
// Sadly slangs types do not include the required information to reliably distinguish between
// Samplers, Textures, Buffers, etc. So we need to define our own types wrapping the underlying
// descriptor types with additional type information.

public interface IBindlessResource
{
    public associatedtype UnderlyingDescriptor : IOpaqueDescriptor;
    public static const bindless::DescriptorKind kind;
}

public struct SampledImage2D : IBindlessResource
{
    public typedef Sampler2D UnderlyingDescriptor;
    public static const bindless::DescriptorKind kind = bindless::DescriptorKind.SampledImage;
}

public struct RWImage2D : IBindlessResource
{
    public typedef RWTexture2D UnderlyingDescriptor;
    public static const bindless::DescriptorKind kind = bindless::DescriptorKind.SampledImage;
}

public struct StorageBuffer<T, L = Std430DataLayout> : IBindlessResource
    where L : IBufferDataLayout
{
    public typedef StructuredBuffer<T, L> UnderlyingDescriptor;
    public static const bindless::DescriptorKind kind = bindless::DescriptorKind.StorageBuffer;
}

public struct RWStorageBuffer<T, L = Std430DataLayout> : IBindlessResource
    where L : IBufferDataLayout
{
    public typedef RWStructuredBuffer<T, L> UnderlyingDescriptor;
    public static const bindless::DescriptorKind kind = bindless::DescriptorKind.StorageBuffer;
}

public struct UniformBuffer<T, L = Std430DataLayout> : IBindlessResource
    where L : IBufferDataLayout
{
    public typedef ConstantBuffer<T, L> UnderlyingDescriptor;
    public static const bindless::DescriptorKind kind = bindless::DescriptorKind.UniformBuffer;
}

public struct Undefined : IBindlessResource
{
    public typedef IOpaqueDescriptor UnderlyingDescriptor;
    public static const bindless::DescriptorKind kind = bindless::DescriptorKind.Unkown;
}

namespace bindless
{
    // Binding slots for different descriptor types
    public static const uint SAMPLED_IMAGE = 0;
    public static const uint STORAGE_IMAGE = 1;
    public static const uint STORAGE_BUFFER = 2;
    public static const uint UNIFORM_BUFFER = 3;

    // Bits for extracting packed handle data
    public static const uint INDEX_BITS = 24;
    public static const uint VERSION_BITS = 6;
    public static const uint TYPE_BITS = 2;

    public static const uint INDEX_MASK = (1 << INDEX_BITS) - 1;
    public static const uint VERSION_MASK = (1 << VERSION_BITS) - 1;
    public static const uint TYPE_MASK = (1 << TYPE_BITS) - 1;

    [allow("parameterBindingsOverlap")] 
    [vk::binding(SAMPLED_IMAGE)] __DynamicResource g_sampledImages[];

    [allow("parameterBindingsOverlap")] 
    [vk::binding(STORAGE_IMAGE)] __DynamicResource g_storageImages[];

    [allow("parameterBindingsOverlap")] 
    [vk::binding(STORAGE_BUFFER)] __DynamicResource g_storageBuffers[];

    [allow("parameterBindingsOverlap")] 
    [vk::binding(UNIFORM_BUFFER)] __DynamicResource g_uniformBuffers[];

    public enum DescriptorKind
    {
        SampledImage,
        StorageImage,
        StorageBuffer,
        UniformBuffer,
        Unkown,
    }

    public struct Handle<T = Undefined> where T : IBindlessResource
    {
        // Packed as: | 2 bits type | 6 bits version | 24 bits index |
        uint handle;

        public __init(uint h)
        {
            handle = h;
        }

        public property uint index
        {
            get { return handle & INDEX_MASK; }
        }

        public property uint version
        {
            get { return (handle >> INDEX_BITS) & VERSION_MASK; }
        }

        public property uint type
        {
            get { return (handle >> (INDEX_BITS + VERSION_BITS)) & TYPE_MASK; }
        }

        public func get() -> T.UnderlyingDescriptor
        {
            if (T.kind == DescriptorKind.SampledImage)
            {
                return g_sampledImages[NonUniformResourceIndex(index)].asOpaqueDescriptor<T.UnderlyingDescriptor>();
            }
            else if (T.kind == DescriptorKind.StorageImage)
            {
                return g_storageImages[NonUniformResourceIndex(index)].asOpaqueDescriptor<T.UnderlyingDescriptor>();
            }
            else if (T.kind == DescriptorKind.StorageBuffer)
            {
                return g_storageBuffers[NonUniformResourceIndex(index)].asOpaqueDescriptor<T.UnderlyingDescriptor>();
            }
            else if (T.kind == DescriptorKind.UniformBuffer)
            {
                return g_uniformBuffers[NonUniformResourceIndex(index)].asOpaqueDescriptor<T.UnderlyingDescriptor>();
            }
            else
            {
                static_assert(false, "Unknown descriptor kind");
                return T.UnderlyingDescriptor();
            }
        }

        public func asHandle<U>() -> Handle<U>
            where U : IBindlessResource
        {
            return Handle<U>(handle);
        }
    };
}