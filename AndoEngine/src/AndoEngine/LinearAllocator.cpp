#include <cstdlib>
#include "AndoEngine/LinearAllocator.h"

LinearAllocatorData::LinearAllocatorData( size_t InCapacity )
: Capacity( InCapacity )
, Used( 0 )
, Peak( 0 )
{
	Data = static_cast<uint8_t*>( std::malloc( Capacity ) );
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