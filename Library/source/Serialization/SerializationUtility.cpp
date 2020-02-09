#include "Serialization/SerializationUtility.h"
#include "Serialization/ByteUtility.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Serialization {
	ScopedDataBlockWrite::ScopedDataBlockWrite(std::ostream& stream)
	: streamPtr(&stream)
	, writeStartPosition(stream.tellp())
	{
		//Write a zero integer to the stream to mark where the size will later be written.
		BlockSizeType const zero = 0;
		WriteLE(&zero, stream);
	}

	ScopedDataBlockWrite::~ScopedDataBlockWrite() {
		//Compute the size of the data that was written after the size marker.
		std::streampos const writeEndPosition = streamPtr->tellp();
		BlockSizeType const blockSize = writeEndPosition - writeStartPosition - sizeof(BlockSizeType);
		//Write the size to the space we reserved earlier
		streamPtr->seekp(writeStartPosition);
		WriteLE(&blockSize, *streamPtr);
		//Go back to where we left off writing so any subsequent data follows this data block
		streamPtr->seekp(writeEndPosition);
	}

	ScopedDataBlockRead::ScopedDataBlockRead(std::istream& stream)
	: streamPtr(&stream)
	{
		BlockSizeType blockSize = 0;
		ReadLE(&blockSize, stream);
		//We have already read the size marker, so the read end position is the current location plus the block size.
		readEndPosition = (stream.tellg() + std::streamoff{blockSize});
	}

	ScopedDataBlockRead::~ScopedDataBlockRead() {
		//Seek ahead to the end position (we may already be there if the entire data block was read)
		streamPtr->seekg(readEndPosition);
	}

	int64_t ScopedDataBlockRead::GetRemainingSize() const {
		return int64_t(readEndPosition - streamPtr->tellg());
	}

	void ScopedDataBlockRead::SkipNestedBlock() const {
		std::streampos const currentPosition = streamPtr->tellg();
		BlockSizeType nestedBlockSize = 0;
		ReadLE(&nestedBlockSize, *streamPtr);
		streamPtr->seekg(std::min(currentPosition + std::streamoff{nestedBlockSize}, readEndPosition));
	}

	void ResetStream(std::stringstream& stream) {
		stream.str("");
		stream.clear();
	}
}
