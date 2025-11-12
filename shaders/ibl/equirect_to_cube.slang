import "modules/constants";
import "modules/ibl";

static const float2 INV_ATAN = float2(0.1591, 0.3183);

[vk::binding(0)] Sampler2D equirectanglarMap;
[vk::binding(1)] RWTexture2DArray<float4> cubeMap;

func sampleEquirectangular(float3 dir) -> float2
{
    // Note: Directions might be shuffled because of Z-up to Y-up conversion
    float2 uv = float2(atan2(dir.y, dir.x), asin(-dir.z));
    uv *= INV_ATAN;
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

    float3 dir = ibl::cubeSampleVector(uv, dispatchID.z);
    float2 equirectUV = sampleEquirectangular(dir);
    float4 color = equirectanglarMap.SampleLevel(equirectUV, 0);

    cubeMap[dispatchID] = color;
}
