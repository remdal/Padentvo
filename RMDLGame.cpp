/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLGame.cpp            +++     +++			**/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 28/10/2025 12:41:46      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "RMDLGame.hpp"

#include "RMDLMathUtils.hpp"

#define IR_RUNTIME_METALCPP
#include <metal_irconverter_runtime/metal_irconverter_runtime.h>

#include <TargetConditionals.h>

bool deviceSupportsResidencySets( MTL::Device* pDevice )
{
    static bool result = false;
    
    static std::once_flag flag;
    std::call_once(flag, [&](){
        NS::OperatingSystemVersion minimumVersion;
        minimumVersion.majorVersion = 15;
        minimumVersion.minorVersion = 0;
        minimumVersion.patchVersion = 0;
        result = NS::ProcessInfo::processInfo()->isOperatingSystemAtLeastVersion(minimumVersion) && pDevice->supportsFamily(MTL::GPUFamilyApple6);
    });
    return (result);
}

RMDLGame::RMDLGame()
: _gameConfig()
, _level(0)
, _prevTargetTimestamp(0.0f)
, _firstFrame(true)
{
    
}

RMDLGame::~RMDLGame()
{
}
