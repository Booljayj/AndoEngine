#include <cstdlib>
#include "Engine/LinearAllocator.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

HeapBuffer::HeapBuffer( size_t InCapacity )
: Capacity(InCapacity)
, PeakUsage(0)
, PeakOverflow(0)
{
	Data = std::make_unique<char[]>(Capacity+1);
	//Data is uninitialized, except for the last byte, which is 0.
	*end() = '\0';
	Current = begin();
}
