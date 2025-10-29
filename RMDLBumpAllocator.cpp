/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLBumpAllocator.cpp            +++     +++	**/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 29/10/2025 14:14:20      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "RMDLBumpAllocator.hpp"

BumpAllocator::BumpAllocator( MTL::Device* pDevice, size_t capacityInBytes, MTL::ResourceOptions resourceOptions )
{
    assert( resourceOptions != MTL::ResourceStorageModePrivate );
    _offset   = 0;
    _capacity = capacityInBytes;
    _pBuffer  = pDevice->newBuffer(capacityInBytes, resourceOptions);
    _contents = (uint8_t*)_pBuffer->contents();
}

BumpAllocator::~BumpAllocator()
{
    _pBuffer->release();
}
