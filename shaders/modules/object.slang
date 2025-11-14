module object;

import modules.bindless;

namespace object
{
    public struct Instance
    {
        float3x4 transform;
        float3 normalRow0;
        public bindless::Handle mesh;
        float3 normalRow1;
        public bindless::Handle material;
        float3 normalRow2;

        public property float4x4 modelMatrix
        {
            get { return float4x4(transform, float4(0.0, 0.0, 0.0, 1.0)); }
        }

        public property float3x3 normalMatrix
        {
            get { return float3x3(normalRow0, normalRow1, normalRow2); }
        }
    };
}