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
	LOG(Temp, Info, "Thread Buffer:{{ Capacity: {}, Current: {}, Peak: {} }}", current->GetCapacity(), current->GetUsed(), current->GetPeakUsage());
}

ScopedThreadBufferMark::ScopedThreadBufferMark() : HeapBuffer::ScopedMark(ThreadBuffer::Get()) {}
