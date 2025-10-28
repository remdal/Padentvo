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
