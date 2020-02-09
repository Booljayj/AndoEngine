#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <ostream>

struct HeapBuffer {
private:
	size_t capacity;
	std::unique_ptr<char[]> data;
	char* current;

	size_t peakUsage;
	size_t peakOverflow;

public:
	HeapBuffer(size_t inCapacity);
	HeapBuffer() = delete;
	HeapBuffer(HeapBuffer&&) = delete;
	HeapBuffer(HeapBuffer const&) = delete;

	inline char* begin() { return data.get(); }
	inline char* end() { return data.get() + capacity; }
	inline char const* begin() const { return data.get(); }
	inline char const* end() const { return data.get() + capacity; }

	inline void Reset() noexcept { current = begin(); }

	inline char* GetCursor() const noexcept { return current; }
	inline void SetCursor(char* newCurrent) noexcept {
		current = std::min(std::max(newCurrent, begin()), end()); //Clamp the input to ensure it's inside the buffer
		peakUsage = std::max(peakUsage, GetUsed());
	}

	inline size_t GetCapacity() const noexcept { return capacity; }
	inline size_t GetAvailable() const noexcept { return end() - current; }
	inline size_t GetUsed() const noexcept { return current - begin(); }
	inline size_t GetPeakUsage() const noexcept { return peakUsage; }
	inline size_t GetPeakOverflow() const noexcept { return peakOverflow; }

	inline bool Contains(void* pointer) const noexcept {
		return true &&
			static_cast<char*>(pointer) >= begin() &&
			static_cast<char*>(pointer) < end();
	}

	/** Returns an aligned array of elements inside this buffer, or nullptr if the buffer does not have enough space */
	template<typename T>
	inline T* Request(size_t count = 1) noexcept {
		static_assert(sizeof(T) > 0, "Template type has zero size");

		if (size_t const requestedBytes = sizeof(T) * count) {
			size_t availableBytes = GetAvailable();
			void* alignedCurrent = GetCursor();
			if (std::align(alignof(T), requestedBytes, alignedCurrent, availableBytes)) {
				SetCursor(static_cast<char*>(alignedCurrent) + requestedBytes);
				return static_cast<T*>(alignedCurrent);
			} else {
				//The actual overflow is probably a bit higher due to alignment, but this number does not need to be exact.
				peakOverflow = std::max(peakOverflow, requestedBytes);
			}
		}
		return nullptr;
	}
};

/** std allocator that uses a buffer to manage allocations. */
template<typename T>
class TLinearAllocator {
	template<typename U>
	friend class TLinearAllocator;

protected:
	HeapBuffer* buffer = nullptr;

public:
	using value_type = T;
	using propagate_on_container_move_assignment = std::true_type;

	TLinearAllocator() = delete;
	TLinearAllocator(HeapBuffer& inBuffer)
	: buffer(&inBuffer)
	{}

	TLinearAllocator(TLinearAllocator const&) = default;

	template<typename U>
	TLinearAllocator(TLinearAllocator<U> const& other)
	: buffer(other.buffer)
	{}

	~TLinearAllocator() = default;

	T* allocate(size_t count, void const* hint = nullptr) {
		if (T* bufferRequest = buffer->Request<T>(count)) {
			return bufferRequest;
		} else {
			//default to heap allocation, we have exceeded the capacity of the temp allocator
			return static_cast<T*>(::operator new(sizeof(T) * count));
		}
	}

	void deallocate(T* pointer, size_t count) {
		if (!buffer->Contains(pointer)) {
			::operator delete(pointer);
		}
	}

	//-----------------------------------
	//Boilerplate for older C++ libraries
	using pointer = T*;
	using const_pointer = T const*;
	using reference = T&;
	using const_reference = T const&;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	template< class U > struct rebind { typedef TLinearAllocator<U> other; };
	using is_always_equal = std::false_type;

	pointer address(reference x) const noexcept { return &x; }
	const_pointer address(const_reference x) const noexcept { return &x; }
	size_type max_size() const { return buffer->GetCapacity(); }

	template<class U, class... ArgTypes>
	void construct(U* pointer, ArgTypes&&... args) {
		::new((void*)pointer) U(std::forward<ArgTypes>(args)...);
	}
	template<class U>
	void destroy(U* pointer) {
		pointer->~U();
	}
};
