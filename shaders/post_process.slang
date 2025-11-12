struct Push
{
    int toneMappingMode;
    float bloomIntensity;
    float exposure;
    float gamma;
}

[vk::push_constant] ConstantBuffer<Push> push;
[vk::binding(0)] RWTexture2D<float4> outFinalMap;
[vk::binding(1)] RWTexture2D<float4> sceneColorMap;
[vk::binding(2)] RWTexture2D<float4> bloomMap;

func ToneMapReinhard(float3 color) -> float3
{
    return color / (color + float3(1.0));
}

func ToneMapACES(float3 color) -> float3
{
    const float A = 2.51;
    const float B = 0.03;
    const float C = 2.43;
    const float D = 0.59;
    const float E = 0.14;
    return clamp((color * (A * color + B)) / (color * (C * color + D) + E), 0.0, 1.0);
}

func applyToneMapping(float3 color) -> float3
{
    switch (push.toneMappingMode)
    {
    case 0:
        return ToneMapReinhard(color);
    case 1:
        return ToneMapACES(color);
    default:
        return color;
    }
}

func applyGammaCorrection(float3 color) -> float3
{
    return pow(color, float3(1.0 / push.gamma));
}

[shader("compute")]
[numthreads(16, 16, 1)]
func main(uint3 dispatchID: SV_DispatchThreadID)
{
    float3 sceneColor = sceneColorMap[dispatchID.xy].rgb;
    float3 bloomColor = bloomMap[dispatchID.xy].rgb;

    float3 color = sceneColor + (push.bloomIntensity * bloomColor);
    color = applyToneMapping(color * push.exposure);
    color = applyGammaCorrection(color);

    outFinalMap[dispatchID.xy] = float4(color, 1.0);
}
