import "modules/constants";

static const float2 invAtan = float2(0.1591, 0.3183);

[vk::binding(0)] Sampler2D equirectanglarMap;
[vk::binding(1)] RWTexture2DArray<float4> cubeMap;

func sampleVector(float2 uv, uint face) -> float3
{
    float3 dir;
    if (face == 0) dir = float3(1.0, uv.y, -uv.x);
    else if (face == 1) dir = float3(-1.0, uv.y, uv.x);
    else if (face == 2) dir = float3(uv.x, 1.0, -uv.y);
    else if (face == 3) dir = float3(uv.x, -1.0, uv.y);
    else if (face == 4) dir = float3(uv.x, uv.y, 1.0);
    else if (face == 5) dir = float3(-uv.x, uv.y, -1.0);
    return normalize(float3(dir.x, -dir.z, dir.y)); // Y-up to Z-up conversion
}

func sampleEquirectangular(float3 dir) -> float2
{
    float2 uv = float2(atan2(dir.z, dir.x), asin(dir.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

[shader("compute")]
[numthreads(16, 16, 1)]
func main(uint3 dispatchID: SV_DispatchThreadID)
{
    uint3 imageSize;
    cubeMap.GetDimensions(imageSize.x, imageSize.y, imageSize.z);

    float2 st = float2(dispatchID.xy) / float2(imageSize.xy);
    float2 uv = float2(st.x, 1.0 - st.y) * 2.0 - 1.0;

    float3 dir = sampleVector(uv, dispatchID.z);
    float2 equirectUV = sampleEquirectangular(dir);
    float4 color = equirectanglarMap.SampleLevel(equirectUV, 0);

    cubeMap[dispatchID] = color;
}
