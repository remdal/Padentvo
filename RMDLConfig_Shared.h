/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLConfig_Shared.h            +++     +++	**/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 27/10/2025 22:25:42      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef RMDLCONFIG_SHARED_H
# define RMDLCONFIG_SHARED_H

# import <simd/simd.h>

constexpr size_t kMaxFramesInFlight = 3;

struct FrameData
{
    simd::float4x4 projectionMatrix;
};



bool deviceSupportsResidencySets( MTL::Device* pDevice );

#endif // RMDLCONFIG_SHARED_H
