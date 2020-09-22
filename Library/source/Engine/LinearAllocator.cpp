#include "Engine/LinearAllocator.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

HeapBuffer::HeapBuffer(size_t inCapacity)
: capacity(inCapacity)
, peakUsage(0)
, peakOverflow(0)
{
	data = std::make_unique<char[]>(capacity+1);
	//Data is uninitialized, except for the last byte, which is always 0.
	*end() = '\0';
	current = begin();
}

void* HeapBuffer::Request(size_t count, size_t size, size_t alignment) noexcept {
	if (size_t const requestedBytes = size * count) {
		size_t availableBytes = GetAvailable();
		void* alignedCurrent = GetCursor();
		if (std::align(alignment, requestedBytes, alignedCurrent, availableBytes)) {
			SetCursor(static_cast<char*>(alignedCurrent) + requestedBytes);
			return alignedCurrent;
		} else {
			//The actual overflow is probably a bit higher due to alignment, but this number does not need to be exact.
			peakOverflow = std::max(peakOverflow, requestedBytes);
		}
	}
	return nullptr;
}
