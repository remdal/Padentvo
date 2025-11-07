/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLSkybox.metal            +++     +++		**/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 07/11/2025 12:52:05      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <metal_stdlib>

using namespace metal;

#include "RMDLMainRenderer_shared.h"

struct SkyboxVertex
{
    float4 pos      [[attribute(0)]];
    float3 normal   [[attribute(2)]];
};

struct SkyboxInOut
{
    float4 pos      [[pos]];
    float3 texcoord;
};

vertex SkyboxInOut skybox_vertex(SkyboxVertex       in              [[ stage_in ]],
                                 constant FrameData &frameData      [[ buffer(2) ]])
{
    SkyboxInOut     out;
    out.pos       = frameData.projection_matrix_center * frameData.sky_modelview_matrix_center * in.pos;
    out.texcoord  = in.normal;
    return out;
}

fragment half4 skybox_fragment(SkyboxInOut          in              [[ stage_in ]],
                               texturecube<float>   skybox_texture  [[ texture(0) ]])
{
    constexpr sampler linearSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);
    float4 color = skybox_texture.sample(linearSampler, in.texcoord);
    return half4(color);
}
