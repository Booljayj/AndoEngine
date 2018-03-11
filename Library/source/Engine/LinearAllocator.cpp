#include <cstdlib>
#include "Engine/LinearAllocator.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

LinearAllocatorData::LinearAllocatorData( size_t Capacity )
: Capacity( Capacity ) //Actual capacity is one less, because we ensure a specific value in the final byte
, Used( 0 )
, Peak( 0 )
{
	Data = static_cast<uint8_t*>( std::malloc( Capacity + 1 ) );
	//because we may want to directly print string literals from allocated memory,
	//we ensure that the final value is always a null terminator. This way it's
	//harder to accidentally go too far and read from arbitrary memory
	Data[Capacity] = '\0';
}

LinearAllocatorData::~LinearAllocatorData()
{
	std::free( static_cast<void*>( Data ) );
}

DESCRIPTION( LinearAllocatorData )
{
	return l_printf(
		CTX.Temp, "[LinearAllocatorData]{ Current: %i/%i, Peak: %i/%i }",
		Value.GetUsed(), Value.GetCapacity(), Value.GetPeak(), Value.GetCapacity()
	);
}
