#pragma once
#include "Engine/StandardTypes.h"

/** General-purpose buffer that keeps track of usage. */
struct Buffer {
public:
	/** A mark which will save a buffer's current cursor position when created, and set the buffer to that position when destroyed. */
	struct ScopedMark {
	public:
		inline ScopedMark(Buffer& inBuffer) : buffer(inBuffer), cursor(inBuffer.GetCursor()) {}
		inline ~ScopedMark() { Pop(); }

		inline void Pop() const { buffer.SetCursor(cursor); }

	private:
		Buffer& buffer;
		char* cursor;
	};

	/** Read-only iteration support for the buffer */
	inline char const* begin() const { return start; }
	inline char const* end() const { return start + capacity; }

	/** Reset the entire buffer, returning the cursor back to the beginning of the buffer */
	inline void Reset() noexcept { current = start; }

	/** The cursor position points to the current location in the buffer where requests can be made */
	inline char* GetCursor() const noexcept { return current; }
	inline void SetCursor(char* newCurrent) noexcept {
		current = std::clamp(newCurrent, start, start + capacity); //Clamp the input to ensure it's valid
		peakUsage = std::max(peakUsage, GetUsed());
	}

	/** Get the full capacity of the buffer, inluding any reserved space */
	inline size_t GetCapacity() const noexcept { return capacity; }
	/** Get the total number of bytes which have been requested from the buffer */
	inline size_t GetUsed() const noexcept { return current - start; }
	/** Get the remaining total number of bytes that can be requested from the buffer */
	inline size_t GetAvailable() const noexcept { return (start + capacity) - current; }

	/** Get the largest total number of bytes which have been requested from the buffer */
	inline size_t GetPeakUsage() const noexcept { return peakUsage; }
	/** Get the largest total number of bytes which were requested and exceeded the size of the buffer */
	inline size_t GetPeakOverflow() const noexcept { return peakOverflow; }

	/** Returns true if the pointer value lies within the buffer */
	inline bool Contains(void* pointer) const noexcept {
		return (static_cast<char*>(pointer) >= begin()) && (static_cast<char*>(pointer) < end());
	}

	/** Returns an aligned array of bytes inside this buffer, or nullptr if the buffer does not have enough space */
	void* Request(size_t count, size_t size, size_t alignment) noexcept;

	/** Returns an aligned array of elements inside this buffer, or nullptr if the buffer does not have enough space */
	template<typename T>
	inline T* Request(size_t count = 1) noexcept {
		static_assert(sizeof(T) > 0, "Template type has zero size");
		return static_cast<T*>(Request(count, sizeof(T), alignof(T)));
	}

protected:
	/** The total usable capacity of the buffer, in bytes */
	const size_t capacity;
	/** The address at the start of the buffer */
	char* start = nullptr;
	/** The current "free" position in the buffer, where memory requests can be made */
	char* current = nullptr;

	/** The largest used amount of the capacity of the buffer over its lifetime */
	size_t peakUsage = 0;
	/** When the capacity of the buffer is exceeded during a request, this is the approximate total amount that was requested */
	size_t peakOverflow = 0;

	Buffer(size_t inCapacity) : capacity(inCapacity) {}
	~Buffer() = default;

	inline void InitStart(char* newStart) {
		start = newStart;
		current = newStart;
	}
};

/** A heap-allocated buffer */
struct HeapBuffer : public Buffer {
public:
	HeapBuffer(size_t inCapacity)
	: Buffer(inCapacity)
	{
		//The actual allocation size is one larger than the capacity, so we can guarantee the final byte is 0
		data = std::make_unique<char[]>(capacity+1);
		InitStart(data.get());
		*(start + capacity) = '\0';
	}

private:
	/** The allocated byte array for this buffer */
	std::unique_ptr<char[]> data;
};

/** A stack-allocated buffer */
template<size_t N>
struct StackBuffer : public Buffer {
public:
	StackBuffer()
	: Buffer(N)
	{
		//The actual allocation size is one larger than the capacity, so we can guarantee the final byte is 0
		InitStart(data);
		*(start + capacity) = '\0';
	}

private:
	char data[N+1];
};
