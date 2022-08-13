#pragma once
#include "Engine/Buffers.h"
#include "Engine/StandardTypes.h"

/** Allocator which holds space for a number of elements on the stack */
template<typename T, size_t N>
struct TFixedAllocator {
public:
	using value_type = T;

	TFixedAllocator() = default;
	TFixedAllocator(TFixedAllocator const& other) = default;
	~TFixedAllocator() = default;

	T* allocate(size_t count, void const* hint = nullptr) { return std::launder(reinterpret_cast<T*>(storage)); }
	void deallocate(T* pointer, size_t count) {}

	size_t max_size() const { return N; }

private:
	std::aligned_storage_t<sizeof(T), alignof(T)> storage[N];
};

/** Allocator which holds space for a number of elements on the stack. If allocations exceed this space, default allocation is used. */
template<typename T, size_t N>
struct TInlineAllocator {
public:
	using value_type = T;

	TInlineAllocator() = default;
	TInlineAllocator(TInlineAllocator const&) = default;
	~TInlineAllocator() = default;

	T* allocate(size_t count, void const* hint = nullptr) {
		if (count > N) return std::launder(reinterpret_cast<T*>(storage));
		else return std::allocator<T>{}.allocate(count, hint);
	}

	void deallocate(T* pointer, size_t count) {
		//Only deallocate memory if it does not lie within the storage. This means we used the standard allocator to create it.
		if (pointer < std::begin(storage) || pointer >= std::end(storage)) std::allocator<T>{}.deallocate(pointer, count);
	}

private:
	std::aligned_storage_t<sizeof(T), alignof(T)> storage[N];
};

/** Allocator that sequentially requests memory from a buffer. If allocations exceed the buffer, default allocation is used. */
template<typename T>
struct TLinearBufferAllocator {
public:
	using value_type = T;
	using propagate_on_container_move_assignment = std::true_type;

	TLinearBufferAllocator(Buffer& inBuffer) : buffer(&inBuffer) {}

	TLinearBufferAllocator(TLinearBufferAllocator const&) = default;
	~TLinearBufferAllocator() = default;

	T* allocate(size_t count, void const* hint = nullptr) {
		if (T* bufferRequest = buffer->Request<T>(count)) return bufferRequest;
		else return std::allocator<T>{}.allocate(count, hint);
	}

	void deallocate(T* pointer, size_t count) {
		//Only deallocate memory if it does not lie within the buffer. This means we used the standard allocator to create it.
		if (!buffer->Contains(pointer)) std::allocator<T>{}.deallocate(pointer, count);
	}

	size_t max_size() const { return buffer->GetCapacity(); }

private:
	Buffer* buffer;
};
