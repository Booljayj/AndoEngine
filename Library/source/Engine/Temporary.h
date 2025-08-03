#pragma once
#include "Engine/Allocators.h"
#include "Engine/Buffers.h"
#include "Engine/Core.h"

/**
 * Temporaries are containers allocated from a per-thread linear buffer. They are fast, cheap, and flexible.
 * All temporaries created within a scope marked with SCOPED_TEMPORARIES() are stable and valid within that scope.
 * When exiting the scope, the memory used by temporaries will be repurposed for new temporaries.
 */

//============================================================
// Buffer types

/** A buffer used for temporary allocations within a thread. Assigned to the thread in which it is created. */
struct ThreadBuffer : public HeapBuffer {
public:
	ThreadBuffer(size_t capacity);
	~ThreadBuffer();

	static inline ThreadBuffer& Get() { return *current; }
	static void LogDebugStats();

private:
	static thread_local ThreadBuffer* current;
};

/** A mark which will save the temporary buffer's current cursor position when created, and set the cursor to that position when destroyed. */
struct ScopedThreadBufferMark : public HeapBuffer::ScopedMark {
	ScopedThreadBufferMark();
};

//============================================================
// Temporary allocator types

/** Allocator used for temporary allocations */
template<typename T>
struct TTemporaryAllocator : public TLinearBufferAllocator<T, std::allocator<T>> {
public:
	TTemporaryAllocator()
		: TLinearBufferAllocator<T, std::allocator<T>>(ThreadBuffer::Get())
	{}

	TTemporaryAllocator(TTemporaryAllocator const&) = default;
	template<typename U> TTemporaryAllocator(TTemporaryAllocator<U> const& other) : TLinearBufferAllocator<T>(other) {}

	template<typename U>
	struct rebind {
		using other = TTemporaryAllocator<U>;
	};
};
