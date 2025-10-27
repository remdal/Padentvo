#ifndef RMDLUTILITIES_H
#define RMDLUTILITIES_H

#include <Foundation/NSError.hpp>
#include <Foundation/NSString.hpp>
#include <iostream>

inline void AAPL_PRINT()
{
    std::cout << std::endl;
}

template< typename T, typename... Args >
void AAPL_PRINT( const T& value, Args&&... args )
{
    std::cout << value << " ";
    AAPL_PRINT( args... );
}

template< typename... Args >
void AAPL_ASSERT( bool condition, Args&&... args )
{
    if ( !condition )
    {
        AAPL_PRINT( args... );
        __builtin_trap();
    }
}

template< typename... Args >
void AAPL_ASSERT_NULL_ERROR( NS::Error* pError, Args&&... args )
{
    if ( pError )
    {
        AAPL_PRINT( args..., pError->localizedDescription()->utf8String() );
        __builtin_trap();
    }
}

template< typename... Args >
void AAPL_ASSERT( NS::Error* pError, Args&&... args ) = delete;

#define AAPLSTR( str ) reinterpret_cast< const NS::String* >(__builtin___CFStringMakeConstantString( str ))

#endif // RMDLUTILITIES_H
