#include "Engine/Temporary.h"
#include "Engine/Logging.h"

thread_local ThreadBuffer* ThreadBuffer::current = nullptr;

ThreadBuffer::ThreadBuffer(size_t capacity) : HeapBuffer(capacity) {
	assert(!current);
	current = this;
}

ThreadBuffer::~ThreadBuffer() {
	current = nullptr;
}

void ThreadBuffer::LogDebugStats() {
	LOGF(Temp, Info, "Thread Buffer:{ Capacity: %i, Current: %i, Peak: %i }", current->GetCapacity(), current->GetUsed(), current->GetPeakUsage());
}

ScopedThreadBufferMark::ScopedThreadBufferMark() : HeapBuffer::ScopedMark(ThreadBuffer::Get()) {}

std::string_view t_printf(char const* format, ...) {
	std::va_list args;
	va_start(args, format);

	Buffer& buffer = ThreadBuffer::Get();
	size_t const available = buffer.GetAvailable();
	char* const begin = buffer.GetCursor();
	size_t const size = vsnprintf(begin, available, format, args);
	//Set the cursor to the byte after the null terminator so subsequent prints don't interfere with the results of this one.
	buffer.SetCursor(begin + size + 1);

	va_end(args);
	return std::string_view{ begin, size };
}
