#pragma once
#include "Engine/Buffers.h"
#include "Engine/STL.h"
#include "Engine/Temporary.h"

/** Version of printf that writes to a provided buffer */
std::string_view l_printf(Buffer& buffer, char const* format, ...);

/** Utility for building a null-terminated string from a number of components. The string is created inside a given buffer. */
struct StringBuilder {
	StringBuilder(Buffer& inBuffer)
	: buffer(inBuffer)
	, start(inBuffer.GetCursor())
	{
		if (buffer.GetAvailable() > 0) *buffer.GetCursor() = '\0';
	}

	/** The size of the string that has been written */
	size_t Size() const { return buffer.GetCursor() - start; }
	/** Get a raw pointer to the string that has been written. This will be null-terminated. */
	char const* Get() const { return start; }
	/** Get a view of the string that has been written. */
	std::string_view GetView() const { return std::string_view{ start, Size() }; }

	/** Write a string to the buffer */
	StringBuilder& operator<<(std::string_view const string);
	/** Write a formatted string to the buffer. Uses the same syntax as printf. */
	StringBuilder& Write(char const* format, ...);

	/** Erase a number of characters from the end of the string */
	inline void Erase(uint32_t count) {
		buffer.SetCursor(buffer.GetCursor() - count);
		*buffer.GetCursor() = '\0';
	}

	/** Replace a character at a specific index */
	inline void Replace(uint32_t index, char replacement) {
		size_t const size = Size();
		if (index < size) start[index] = replacement;
	}

	/** Replace the last character that was written. Useful for lists, to remove a trailing separator. */
	inline void ReplaceLast(char replacement) {
		size_t const size = Size();
		if (size > 0) start[size - 1] = replacement;
	}

private:
	Buffer& buffer;
	char* start;
};

/**
 * Utility for building a null-terminated string from a number of components.
 * The string is created inside the temporary buffer, and temporary memory is consumed during each write.
 */
struct TemporaryStringBuilder : public StringBuilder {
public:
	TemporaryStringBuilder()
	: StringBuilder(*threadTemporaryBuffer)
	{}
};
