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
	alignas(T) std::byte storage[sizeof(T) * N];
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
	alignas(T) std::byte storage[sizeof(T) * N];
};

/** Allocator that sequentially requests memory from a buffer. If allocations exceed the buffer, default allocation is used. */
template<typename T, typename FallbackAllocatorType = std::allocator<T>>
struct TLinearBufferAllocator {
public:
	template<typename U, typename OtherAlloc> friend struct TLinearBufferAllocator;

	using value_type = T;
	using propagate_on_container_move_assignment = std::true_type;

	TLinearBufferAllocator(Buffer& inBuffer) : buffer(&inBuffer) {}

	TLinearBufferAllocator(TLinearBufferAllocator const&) = default;
	template<class U> TLinearBufferAllocator(const TLinearBufferAllocator<U, typename std::allocator_traits<FallbackAllocatorType>::template rebind_alloc<U>>& other) noexcept : buffer(other.buffer), fallback(other.fallback) {}
	~TLinearBufferAllocator() = default;

	template<class U> bool operator==(const TLinearBufferAllocator<U>& other) const noexcept { return buffer == other.buffer; }
	template<class U> bool operator!=(const TLinearBufferAllocator<U>& other) const noexcept { return !this->operator==(other); }

	T* allocate(size_t count) {
		if (T* bufferRequest = buffer->Request<T>(count)) return bufferRequest;
		else return fallback.allocate(count);
	}

	void deallocate(T* const pointer, size_t count) {
		//Only deallocate memory if it does not lie within the buffer. This means we used the fallback allocator to create it.
		if (!buffer->Contains(pointer)) fallback.deallocate(pointer, count);
	}

	size_t max_size() const { return buffer->GetCapacity(); }

	template<typename U>
	struct rebind {
		using other = TLinearBufferAllocator<U, typename std::allocator_traits<FallbackAllocatorType>::template rebind_alloc<U>>;
	};

private:
	Buffer* buffer;
	FallbackAllocatorType fallback;
};
