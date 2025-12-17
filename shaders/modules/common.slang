module common;

import modules.bindless;

namespace common
{
    public struct Frustum
    {
        public float4 planes[6];
    }

    public struct Camera
    {
        public float4x4 view;
        public float4x4 projection;
        public float4x4 viewProjection;
        public Frustum frustum;
        public float3 position;
    }

    public struct Vertex
    {
        public float3 position;
        public float3 normal;
        public float2 uv;
        public float3 color;
    };

    public struct Meshlet
    {
        public float3 center;
        public float radius;
        public int8_t coneAxis[3];
        public int8_t coneCutoff;
        public uint vertexOffset;
        public uint primitiveOffset;
        public uint8_t vertexCount;
        public uint8_t primitiveCount;
    };

    public struct Mesh
    {
        public bindless::Handle<StorageBuffer<Vertex, ScalarDataLayout>> vertex;
        public bindless::Handle<StorageBuffer<uint>> index;
        public bindless::Handle<StorageBuffer<Meshlet>> meshlet;
        public bindless::Handle<StorageBuffer<uint>> meshletIndex;
        public bindless::Handle<StorageBuffer<uint8_t, ScalarDataLayout>> meshletPrimitive;
        public uint vertexCount;
        public uint indexCount;
        public uint meshletCount;
    };

    public struct VertexIn
    {
        [vk::location(0)] public float3 position;
        [vk::location(1)] public float3 normal;
        [vk::location(2)] public float2 uv;
        [vk::location(3)] public float3 color;
    };

    public struct GBuffer
    {
        [vk::location(0)] public float4 position;
        [vk::location(1)] public float4 normal;
        [vk::location(2)] public float4 albedo;
        [vk::location(3)] public float4 arm;
        [vk::location(4)] public float4 emissive;
    };
}
