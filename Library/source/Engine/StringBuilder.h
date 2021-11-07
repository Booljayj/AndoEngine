#include "Engine/STL.h"
#include "Engine/LinearAllocator.h"

/** Utility for building a null-terminated string from a number of components, where the maximum size of the string is not known. */
struct StringBuilder {
	StringBuilder(HeapBuffer& inBuffer)
	: buffer(inBuffer)
	, string(inBuffer.GetCursor())
	{
		*buffer.GetCursor() = '\0';
	}
	StringBuilder() : StringBuilder(*threadHeapBuffer) {}

	/** The size of the string that has been written */
	size_t Size() const { return buffer.GetCursor() - string; }
	/** Get a raw pointer to the string that has been written. This will be null-terminated. */
	char const* Get() const { return string; }
	/** Get a view of the string that has been written. */
	std::string_view GetView() const { return std::string_view{ string, Size() }; }

	/** Write a string to the buffer */
	inline StringBuilder& operator<<(std::string_view const string) {
		size_t const available = buffer.GetAvailable();
		char* const begin = buffer.GetCursor();
		size_t const size = std::min(available, string.size());

		std::memcpy(begin, string.data(), size);
		*(begin + size) = '\0';
		//Move the cursor over the null terminator, so the next written string will overwrite it.
		buffer.SetCursor(begin + size);
		return *this;
	}

	/** Write a formatted string to the buffer */
	inline StringBuilder& Write(char const* format, ...) {
		std::va_list args;
		va_start(args, format);

		size_t const available = buffer.GetAvailable();
		char* const begin = buffer.GetCursor();
		size_t const size = vsnprintf(begin, available, format, args);
		//Move the cursor over the null terminator, so the next written string will overwrite it.
		buffer.SetCursor(begin + size);

		va_end(args);
		return *this;
	}

	/** Erase a number of characters from the end of the string */
	inline void Erase(uint32_t count) {
		buffer.SetCursor(buffer.GetCursor() - count);
		*buffer.GetCursor() = '\0';
	}

	/** Replace a character at a specific index */
	inline void Replace(uint32_t index, char replacement) {
		size_t const size = Size();
		if (index < size) string[index] = replacement;
	}

	/** Replace the last character that was written. Useful for lists, to remove a trailing separator. */
	inline void ReplaceLast(char replacement) {
		size_t const size = Size();
		if (size > 0) string[size - 1] = replacement;
	}

private:
	HeapBuffer& buffer;
	char* string;
};

/** Utility for building a null-terminated string from a number of components, where the maximum size of the string is known. */
template<uint32_t MaxSize>
struct FixedStringBuilder {
	using BufferType = std::array<char, MaxSize + 1>;

	/** The size of the string that has been written */
	uint32_t Size() const { return used; }
	/** Get a raw pointer to the string that has been written. This will be null-terminated. */
	char const* Get() const { return buffer.data(); }
	/** Get a view of the string that has been written. */
	std::string_view GetView() const { return std::string_view{ buffer.data(), used }; }
	/** Get read-only access to the underlying buffer */
	BufferType const& GetBuffer() const { return buffer; }

	/** Write a string to the buffer */
	inline FixedStringBuilder& operator<<(std::string_view const string) {
		if ((Size() + string.size()) <= MaxSize) {
			std::memcpy(buffer.data() + used, string.data(), string.size());
			used += string.size();
		}
		return *this;
	}

	/** Write a formatted string to the buffer */
	inline void Write(char const* format, ...) {
		std::va_list args;
		va_start(args, format);

		size_t const available = MaxSize - used;
		char* const begin = buffer.data() + used;
		size_t const size = vsnprintf(begin, available, format, args);
		used += size;

		va_end(args);
	}

	/** Erase a number of characters from the end of the string */
	inline void Erase(uint32_t count) {
		used -= std::min(count, Size());
		std::memset(buffer + used, 0, MaxSize - used);
	}

	/** Replace a character at a specific index */
	inline void Replace(uint32_t index, char replacement) {
		if (index < Size()) buffer[index] = replacement;
	}

	/** Replace the last character that was written. Useful for lists, to remove a trailing separator. */
	inline void ReplaceLast(char replacement) {
		if (Size() > 0) buffer[Size() - 1] = replacement;
	}

private:
	BufferType buffer;
	uint32_t used = 0;
};
