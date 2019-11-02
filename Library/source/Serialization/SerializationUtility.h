#pragma once
#include "Serialization/Serializer.h"
#include <ostream>
#include <sstream>

namespace Reflection {
	struct TypeInfo;
}

namespace Serialization {
	/** Type used to store the size of a data block */
	using BlockSizeType = uint32_t;

	/**
	 * Opens a scope where a binary data block will be written to the stream.
	 * Writes block metadata and seeks to the next write position when destructed.
	 */
	struct ScopedDataBlockWrite {
		ScopedDataBlockWrite(std::ostream& Stream);
		~ScopedDataBlockWrite();

	private:
		std::ostream* StreamPtr;
		std::streampos WriteStartPosition;
	};

	/**
	 * Opens a scope where a binary data block will be read from the stream.
	 * Seeks ahead to the next read position when destructed.
	 * Can be used to keep track of the read, including how much more data is available from the data block.
	 */
	struct ScopedDataBlockRead {
		ScopedDataBlockRead(std::istream& Stream);
		~ScopedDataBlockRead();

		/** Get the number of bytes that remain to be read from the stream */
		int64_t GetRemainingSize() const;
		/** Skip a nested data block from within the stream. Will read the block size and seek ahead to the end position */
		void SkipNestedBlock() const;

	private:
		std::istream* StreamPtr;
		std::streampos ReadEndPosition;
	};

	/** Reset the stringstream contents */
	void ResetStream(std::stringstream& Stream);
}
