#include <cstdlib>
#include "Engine/LinearAllocator.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

HeapBuffer::HeapBuffer( size_t inCapacity )
: capacity(inCapacity)
, peakUsage(0)
, peakOverflow(0)
{
	data = std::make_unique<char[]>(capacity+1);
	//Data is uninitialized, except for the last byte, which is always 0.
	*end() = '\0';
	current = begin();
}
