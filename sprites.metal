#include <metal_stdlib>
using namespace metal;

struct FrameData {
    float4x4 projectionMx;
};

struct InstancePositions {
    float4 position;
};

struct VertexIn {
    float4 position [[attribute(0)]];
    float4 texcoord [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]];
    float4 color;
    float2 uv0;
    float arrayIndex;
};

vertex VertexOut MainVS(VertexIn vin               [[stage_in]],
                        constant FrameData& frame  [[buffer(0)]],
                        const device InstancePositions* instancePositions [[buffer(1)]],
                        uint instanceID            [[instance_id]])
{
    VertexOut out;
    
    float3 instancePos = instancePositions[instanceID].position.xyz;
    out.position = frame.projectionMx * float4(vin.position.xy + instancePos.xy, 0.0, 1.0);
    out.color = float4(1.0, 1.0, 1.0, 0.0);
    out.uv0 = vin.texcoord.xy;
    out.arrayIndex = instancePos.z;
    return out;
}

fragment half4 MainFS(VertexOut in                 [[stage_in]],
                      texture2d_array<half> texArr [[texture(0)]],
                      sampler samp                 [[sampler(0)]])
{
    float layerIndex = in.arrayIndex;
    half4 texel = texArr.sample(samp, float3(in.uv0, layerIndex));
    texel.rgb *= texel.a;
    return texel;
}

