#include <cstdlib>
#include "Engine/LinearAllocator.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

HeapBuffer::HeapBuffer( size_t Capacity )
: Capacity( Capacity )
, Used( 0 )
, Peak( 0 )
{
	Data = static_cast<uint8_t*>( std::malloc( Capacity + 1 ) );
	Data[Capacity] = '\0'; //zero terminate the final byte
}

HeapBuffer::~HeapBuffer()
{
	std::free( static_cast<void*>( Data ) );
}
