#include "Engine/Buffers.h"

void* Buffer::Request(size_t count, size_t size, size_t alignment) noexcept {
	if (size_t const numRequestedBytes = size * count) {
		size_t available = GetAvailable();
		void* cursor = GetCursor();
		if (std::align(alignment, numRequestedBytes, cursor, available)) {
			SetCursor(static_cast<char*>(cursor) + numRequestedBytes);
			return cursor;
		} else {
			//The actual overflow is probably a bit higher due to alignment, but this number does not need to be exact.
			peakOverflow = std::max(peakOverflow, numRequestedBytes);
		}
	}
	return nullptr;
}
