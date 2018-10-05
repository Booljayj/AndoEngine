#include <cstdlib>
#include "Engine/LinearAllocator.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

HeapBuffer::HeapBuffer( size_t Capacity )
: PeakUsage( 0 )
{
	Begin = static_cast<char*>( std::malloc( Capacity + 1 ) );
	End = Begin + Capacity;
	*End = '\0'; //zero terminate the final byte
	Current = Begin;
}

HeapBuffer::~HeapBuffer()
{
	std::free( static_cast<void*>( Begin ) );
}
