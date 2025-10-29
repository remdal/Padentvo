/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLBumpAllocator.hpp            +++     +++	**/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 29/10/2025 14:14:19      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef RMDLBUMPALLOCATOR_HPP
# define RMDLBUMPALLOCATOR_HPP

#include <Metal/Metal.hpp>
#include <cassert>
#include <cstdint>
#include <tuple>

namespace mem
{
constexpr uint64_t alignUp(uint64_t n, uint64_t alignment)
{
    return (n + alignment - 1) & ~(alignment - 1);
}
}

class BumpAllocator
{
public:
    BumpAllocator( MTL::Device* pDevice, size_t capacityInBytes, MTL::ResourceOptions resourceOptions );
    ~BumpAllocator();

    void reset() { _offset = 0; }

    template <typename T>
    std::pair<T*, uint64_t> allocate(uint64_t count = 1) noexcept
    {
        uint64_t allocSize = mem::alignUp(sizeof(T) * count, 8);
        assert((_offset + allocSize) <= _capacity);
        T* dataPtr          = reinterpret_cast<T*>(_contents + _offset);
        uint64_t dataOffset = _offset;
        _offset += allocSize;
        return { dataPtr, dataOffset };
    }
    MTL::Buffer* baseBuffer() const noexcept
    {
        return (_pBuffer);
    }

private:
    MTL::Buffer*    _pBuffer;
    uint64_t        _offset;
    uint64_t        _capacity;
    uint8_t*        _contents;
};

#endif
