#pragma once
#include "Engine/STL.h"

/** A heap-allocated data buffer in which memory can be requested. Memory is handled in a stack-based manner */
struct HeapBuffer {
private:
	/** The total capacity of the buffer, in bytes */
	size_t capacity;
	/** The allocated byte array for this buffer */
	std::unique_ptr<char[]> data;
	/** The current "free" position in the buffer, where memory requests can be made */
	char* current;

	/** The largest used amount of the capacity of the buffer over its lifetime */
	size_t peakUsage;
	/** When the capacity of the buffer is exceeded during a request, this is the amount that was requested */
	size_t peakOverflow;

public:
	/** A mark which will save a buffer's current cursor position when created, and set the buffer to that position when destroyed. */
	struct Mark {
		inline Mark(HeapBuffer& inBuffer)
		: buffer(&inBuffer)
		, cursor(inBuffer.GetCursor())
		{}

		inline ~Mark() {
			Pop();
		}

		inline void Pop() const {
			buffer->SetCursor(cursor);
		}

	private:
		HeapBuffer* buffer;
		char* cursor;
	};

	HeapBuffer(size_t inCapacity);
	HeapBuffer() = delete;
	HeapBuffer(HeapBuffer&&) = delete;
	HeapBuffer(HeapBuffer const&) = delete;

	inline char* begin() { return data.get(); }
	inline char* end() { return data.get() + capacity; }
	inline char const* begin() const { return data.get(); }
	inline char const* end() const { return data.get() + capacity; }

	/** Reset the entire buffer, returning the cursor back to the beginning of the buffer */
	inline void Reset() noexcept { current = begin(); }

	/** The cursor position points to the current location in the buffer where requests can be made. It can move anywhere within the capacity of the buffer */
	inline char* GetCursor() const noexcept { return current; }
	inline void SetCursor(char* newCurrent) noexcept {
		current = std::clamp(newCurrent, begin(), end()); //Clamp the input to ensure it's inside the buffer
		peakUsage = std::max(peakUsage, GetUsed());
	}

	inline size_t GetCapacity() const noexcept { return capacity; }
	inline size_t GetAvailable() const noexcept { return end() - current; }
	inline size_t GetUsed() const noexcept { return current - begin(); }
	inline size_t GetPeakUsage() const noexcept { return peakUsage; }
	inline size_t GetPeakOverflow() const noexcept { return peakOverflow; }

	/** Returns true if the pointer value lies within the buffer */
	inline bool Contains(void* pointer) const noexcept {
		return true &&
			static_cast<char*>(pointer) >= begin() &&
			static_cast<char*>(pointer) < end();
	}

	/** Returns an aligned array of elements inside this buffer, or nullptr if the buffer does not have enough space */
	template<typename T>
	inline T* Request(size_t count = 1) noexcept {
		static_assert(sizeof(T) > 0, "Template type has zero size");
		return static_cast<T*>(Request(count, sizeof(T), alignof(T)));
	}

	void* Request(size_t count, size_t size, size_t alignment) noexcept;
};

/** The pointer to the heap buffer for temporary allocations in each thread. */
extern thread_local HeapBuffer* threadHeapBuffer;

/** std allocator that uses a linear buffer to manage allocations. If not provided a buffer, it will use the thread-local HeapBuffer. */
template<typename T>
class TLinearAllocator {
	template<typename U>
	friend class TLinearAllocator;

public:
	using value_type = T;
	using propagate_on_container_move_assignment = std::true_type;

	TLinearAllocator() : buffer(threadHeapBuffer) {}
	TLinearAllocator(HeapBuffer& inBuffer) : buffer(&inBuffer) {}
	TLinearAllocator(TLinearAllocator const&) = default;
	template<typename U>
	TLinearAllocator(TLinearAllocator<U> const& other) : buffer(other.buffer) {}
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

private:
	HeapBuffer* buffer;
};
