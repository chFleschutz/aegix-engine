struct Push
{
    float filterScale;
}

[vk::push_constant] ConstantBuffer<Push> push;

[vk::binding(0)] RWTexture2D<float4> outputImage;
[vk::binding(1)] Sampler2D inputMipMap;

[shader("compute")]
[numthreads(16, 16, 1)]
func main(uint3 dispatchID: SV_DispatchThreadID)
{
    uint2 outputSize;
    outputImage.GetDimensions(outputSize.x, outputSize.y);
    float2 texelSize = 1.0 / float2(outputSize);
    float2 uv = (float2(dispatchID.xy) + 0.5) * texelSize;

    float2 step = texelSize * push.filterScale;

    // Take 9 samples around the current texel (e)
    // a - b - c
    // d - e - f
    // g - h - i
    float3 a = inputMipMap.SampleLevel(float2(uv.x - step.x, uv.y - step.y), 0).rgb;
    float3 b = inputMipMap.SampleLevel(float2(uv.x,          uv.y - step.y), 0).rgb;
    float3 c = inputMipMap.SampleLevel(float2(uv.x + step.x, uv.y - step.y), 0).rgb;
    float3 d = inputMipMap.SampleLevel(float2(uv.x - step.x, uv.y), 0).rgb;
    float3 e = inputMipMap.SampleLevel(float2(uv.x,          uv.y), 0).rgb;
    float3 f = inputMipMap.SampleLevel(float2(uv.x + step.x, uv.y), 0).rgb;
    float3 g = inputMipMap.SampleLevel(float2(uv.x - step.x, uv.y + step.y), 0).rgb;
    float3 h = inputMipMap.SampleLevel(float2(uv.x,          uv.y + step.y), 0).rgb;
    float3 i = inputMipMap.SampleLevel(float2(uv.x + step.x, uv.y + step.y), 0).rgb;

    // Apply 3x3 gaussian filter weights
    //        | 1 2 1 |
    // 1/16 * | 2 4 2 |
    //        | 1 2 1 |
    float3 bloomColor = (a + c + g + i) * 0.0625 + (b + d + f + h) * 0.125 + e * 0.25;
    outputImage[dispatchID.xy] = float4(bloomColor, 1.0);
}
