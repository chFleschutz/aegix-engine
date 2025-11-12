import "modules/constants";
import "modules/ibl";

static const uint SAMPLE_COUNT = 16 * 1024;
static const float INV_SAMPLE_COUNT = 1.0 / float(SAMPLE_COUNT);
static const float3 MAX_BRIGHTNESS = float3(1000.0);

[vk::binding(0)] SamplerCube environmentMap;
[vk::binding(1)] RWTexture2DArray<float4> irradianceMap;

func computeBasisVectors(const float3 N, out float3 S, out float3 T)
{
    // Branchless non-degenerate tangent
    T = cross(N, float3(0.0, 1.0, 0.0));
    T = lerp(cross(N, float3(1.0, 0.0, 0.0)), T, step(constants::EPSILON, dot(T, T)));
    T = normalize(T);
    S = cross(N, T);
}

func sampleHemisphere(float2 x) -> float3
{
    // Cosine weighted importance sampling
    float phi = 2.0 * constants::PI * x.y;
    float cosTheta = x.x;
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

func hemisphereToWorld(const float3 v, const float3 N, const float3 S, const float3 T) -> float3
{
    return v.x * S + v.y * T + v.z * N;
}

[shader("compute")]
[numthreads(16, 16, 1)]
func main(uint3 dispatchID: SV_DispatchThreadID)
{
    uint3 imageSize;
    irradianceMap.GetDimensions(imageSize.x, imageSize.y, imageSize.z);

    float2 st = float2(dispatchID.xy) / float2(imageSize.xy);
    float2 uv = float2(st.x, 1.0 - st.y) * 2.0 - 1.0;

    float3 N = ibl::cubeSampleVector(uv, dispatchID.z);
    float3 S, T;
    computeBasisVectors(N, S, T);

    float3 irradiance = float3(0.0);
    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        float2 pos = ibl::hammersley(i, SAMPLE_COUNT);
        float3 L = hemisphereToWorld(sampleHemisphere(pos), N, S, T);
        float NdotL = max(dot(N, L), 0.0);

        // Clamp color to reduces noise from overly bright pixels
        float3 sampleColor = min(environmentMap.SampleLevel(L, 0).rgb, MAX_BRIGHTNESS);
        irradiance += 2.0 * sampleColor * NdotL;
    }
    irradiance /= float3(SAMPLE_COUNT);
    irradianceMap[dispatchID] = float4(irradiance, 1.0);
}
