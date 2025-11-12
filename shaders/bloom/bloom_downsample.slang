import "modules/constants";

struct Push
{
    int mipLevel;
    float filterScale;
}

[vk::binding(0)] RWTexture2D<float4> outputImage;
[vk::binding(1)] Sampler2D inputMipMap;

[vk::push_constant] ConstantBuffer<Push> push;

func toSRGB(float3 color) -> float3
{
    return pow(color, float3(constants::INV_GAMMA));
}

func luminance(float3 color) -> float
{
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}

func karisAverage(float3 color) -> float3
{
    return 1.0 / (1.0 + luminance(toSRGB(color)) * 0.25);
}

[shader("compute")]
[numthreads(16, 16, 1)]
func main(uint3 dispatchID: SV_DispatchThreadID)
{
    float2 imageSize;
    outputImage.GetDimensions(imageSize.x, imageSize.y);
    float2 texelSize = 1.0 / imageSize;
    float2 uv = (float2(dispatchID.xy) + 0.5) * texelSize;

    float2 step = texelSize * push.filterScale;

    // Take 13 samples around the current texel (e)
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    float3 a = inputMipMap.SampleLevel(float2(uv.x - 2.0 * step.x, uv.y - 2.0 * step.y), 0).rgb;
    float3 b = inputMipMap.SampleLevel(float2(uv.x,                uv.y - 2.0 * step.y), 0).rgb;
    float3 c = inputMipMap.SampleLevel(float2(uv.x + 2.0 * step.x, uv.y - 2.0 * step.y), 0).rgb;
    float3 d = inputMipMap.SampleLevel(float2(uv.x - 2.0 * step.x, uv.y), 0).rgb;
    float3 e = inputMipMap.SampleLevel(float2(uv.x,                uv.y), 0).rgb;
    float3 f = inputMipMap.SampleLevel(float2(uv.x + 2.0 * step.x, uv.y), 0).rgb;
    float3 g = inputMipMap.SampleLevel(float2(uv.x - 2.0 * step.x, uv.y + 2.0 * step.y), 0).rgb;
    float3 h = inputMipMap.SampleLevel(float2(uv.x,                uv.y + 2.0 * step.y), 0).rgb;
    float3 i = inputMipMap.SampleLevel(float2(uv.x + 2.0 * step.x, uv.y + 2.0 * step.y), 0).rgb;
    float3 j = inputMipMap.SampleLevel(float2(uv.x - step.x,       uv.y - step.y), 0).rgb;
    float3 k = inputMipMap.SampleLevel(float2(uv.x + step.x,       uv.y - step.y), 0).rgb;
    float3 l = inputMipMap.SampleLevel(float2(uv.x - step.x,       uv.y + step.y), 0).rgb;
    float3 m = inputMipMap.SampleLevel(float2(uv.x + step.x,       uv.y + step.y), 0).rgb;

    // Apply weighted average
    //        | 1 - 2 - 1 |
    //        | - 4 - 4 - |
    // 1/32 * | 2 - 4 - 2 |
    //        | - 4 - 4 - |
    //        | 1 - 2 - 1 |
    float3 color = float3(0.0);
    switch (push.mipLevel)
    {
    case 0:
    {
        // Use Karis Average on first mip level to filter fireflies
        float3 group1 = (a + b + d + e) * (0.125 / 4.0);
        float3 group2 = (b + c + e + f) * (0.125 / 4.0);
        float3 group3 = (d + e + g + h) * (0.125 / 4.0);
        float3 group4 = (e + f + h + i) * (0.125 / 4.0);
        float3 group5 = (j + k + l + m) * (0.5 / 4.0);
        group1 *= karisAverage(group1);
        group2 *= karisAverage(group2);
        group3 *= karisAverage(group3);
        group4 *= karisAverage(group4);
        group5 *= karisAverage(group5);
        color = group1 + group2 + group3 + group4 + group5;
        break;
    }
    default:
    {
        color = e * 0.125;
        color += (a + c + g + i) * 0.03125;
        color += (b + d + f + h) * 0.0625;
        color += (j + k + l + m) * 0.125;
        break;
    }
    }

    color = max(color, constants::EPSILON);
    outputImage[dispatchID.xy] = float4(color, 1.0);
}
