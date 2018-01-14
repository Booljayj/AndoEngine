#include <cstdlib>
#include "Engine/LinearAllocator.h"

LinearAllocatorData::LinearAllocatorData( size_t RealCapacity )
: Capacity( RealCapacity - 1 ) //Actual capacity is one less, because we ensure a specific final value
, Used( 0 )
, Peak( 0 )
{
	Data = static_cast<uint8_t*>( std::malloc( RealCapacity ) );
	//because we may want to directly print string literals from allocated memory,
	//we ensure that the final value is always a null terminator. This way it's
	//not possible to accidentally go too far and read from arbitrary memory
	*(Data + Capacity) = '\0';
	assert( Data != nullptr );
}

LinearAllocatorData::~LinearAllocatorData()
{
	std::free( static_cast<void*>( Data ) );
}

std::ostream& operator<<( std::ostream& Stream, LinearAllocatorData& Alloc )
{
	Stream << "[LinearAllocatorData]{ data: " << reinterpret_cast<void*>( Alloc.Data );
	Stream << ", usage: " << Alloc.Used << "/" << Alloc.Capacity << ", peak: " << Alloc.Peak << " }";
	return Stream;
}