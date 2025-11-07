import "../modules/geometry";
import "../modules/constants";
import "../modules/tbn";
import "../modules/bindless";

static const uint NUM_THREADS = 32;
static const uint MAX_VERTICES = 64;
static const uint MAX_PRIMITIVES = 126;

struct MSOut
{
    float4 position : SV_Position;
    float3 worldPosition;
    float3 worldNormal;
    float2 uv;
};

struct Global
{
    float4x4 projection;
    float4x4 view;
    float4x4 inverseView;
};

struct Material
{
    float3 albedo;
    float3 emissive;
    float metallic;
    float roughness;
    float ao;
    bindless::Handle albedoMap;
    bindless::Handle normalMap;
    bindless::Handle metalRoughnessMap;
    bindless::Handle aoMap;
    bindless::Handle emissiveMap;
};

struct Mesh
{
    bindless::Handle vertex;
    bindless::Handle meshlet;
    bindless::Handle meshletIndex;
    bindless::Handle meshletPrimitive;
};

struct Push
{
    float4x4 model;
    float4x4 normal;
    bindless::Handle global;
    bindless::Handle mesh;
    bindless::Handle material;
};

[vk::push_constant] ConstantBuffer<Push> push;

[vk::binding(bindless::STORAGE_BUFFER)] StructuredBuffer<geometry::Meshlet> meshletBuffer[];
[vk::binding(bindless::STORAGE_BUFFER)] StructuredBuffer<uint> indexBuffer[];
[vk::binding(bindless::STORAGE_BUFFER)] StructuredBuffer<uint8_t, ScalarDataLayout> primitiveBuffer[];
[vk::binding(bindless::STORAGE_BUFFER)] StructuredBuffer<geometry::Vertex, ScalarDataLayout> vertexBuffer[];

[vk::binding(bindless::UNIFORM_BUFFER)] ConstantBuffer<Global, Std430DataLayout> globalBuffer[];
[vk::binding(bindless::UNIFORM_BUFFER)] ConstantBuffer<Material, Std430DataLayout> materialBuffer[];
[vk::binding(bindless::UNIFORM_BUFFER)] ConstantBuffer<Mesh, Std430DataLayout> meshBuffer[];

// Mesh Shader --------------------

[shader("mesh")]
[numthreads(NUM_THREADS, 1, 1)]
[outputtopology("triangle")]
func meshMain(
    uint3 groupID: SV_GroupID,
    uint3 threadID: SV_GroupThreadID,
    out vertices MSOut meshVertices[MAX_VERTICES],
    out indices uint3 meshPrimitives[MAX_PRIMITIVES])
{
    Global global = push.global.get(globalBuffer);
    Mesh mesh = push.mesh.get(meshBuffer);
    geometry::Meshlet meshlet = mesh.meshlet.get(meshletBuffer)[groupID.x];

    SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

    float4x4 viewProjection = mul(global.projection, global.view);

    // Emit vertices
    for (uint i = threadID.x; i < uint(meshlet.vertexCount); i += NUM_THREADS)
    {
        uint index = mesh.meshletIndex.get(indexBuffer)[meshlet.vertexOffset + i];
        let v = mesh.vertex.get(vertexBuffer)[index];

        float4 worldPos = mul(push.model, float4(v.position, 1.0));
        meshVertices[i].position = mul(viewProjection, worldPos);
        meshVertices[i].worldPosition = worldPos.xyz;
        meshVertices[i].worldNormal = normalize(mul(float3x3(push.normal), v.normal));
        meshVertices[i].uv = v.uv;
    }

    // Emit primitives
    for (uint i = threadID.x; i < uint(meshlet.primitiveCount); i += NUM_THREADS)
    {
        uint offset = meshlet.primitiveOffset + i * 3;
        meshPrimitives[i] = uint3(
            mesh.meshletPrimitive.get(primitiveBuffer)[offset + 0], 
            mesh.meshletPrimitive.get(primitiveBuffer)[offset + 1],
            mesh.meshletPrimitive.get(primitiveBuffer)[offset + 2]);
    }
}

// Fragment Shader --------------------

[shader("fragment")]
func fragmentMain(MSOut input, out geometry::GBuffer output)
{
    let mat = push.material.get(materialBuffer);

    float3 albedo = mat.albedoMap.Sample(input.uv).rgb * mat.albedo;
    float3 normal = mat.normalMap.Sample(input.uv).rgb * 2.0 - 1.0;
    float3 emissive = mat.emissiveMap.Sample(input.uv).rgb * mat.emissive;
    float2 metalicRoughness = mat.metalRoughnessMap.Sample(input.uv).bg;
    float metallic = metalicRoughness.r * mat.metallic;
    float roughness = metalicRoughness.g * mat.roughness;
    float ao = mat.aoMap.Sample(input.uv).r * mat.ao;

    let TBN = TBN::calcMatrix(input.worldPosition, input.worldNormal, input.uv);
    float3 N = normalize(length(normal) < 0.1 ? input.worldNormal : mul(normal, TBN));

    output.position = float4(input.worldPosition, 1.0);
    output.normal = float4(N, 0.0);
    output.albedo = float4(albedo, 1.0);
    output.arm = float4(ao, roughness, metallic, 0.0);
    output.emissive = float4(emissive, 1.0);
}
