#include "Serialization/SerializationUtility.h"
#include "Serialization/ByteUtility.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Serialization {
	ScopedDataBlockWrite::ScopedDataBlockWrite(std::ostream& Stream)
	: StreamPtr(&Stream)
	, WriteStartPosition(Stream.tellp())
	{
		//Write a zero integer to the stream to mark where the size will later be written.
		BlockSizeType const Zero = 0;
		WriteLE(&Zero, Stream);
	}

	ScopedDataBlockWrite::~ScopedDataBlockWrite() {
		//Compute the size of the data that was written after the size marker.
		std::streampos const WriteEndPosition = StreamPtr->tellp();
		BlockSizeType const BlockSize = WriteEndPosition - WriteStartPosition - sizeof(BlockSizeType);
		//Write the size to the space we reserved earlier
		StreamPtr->seekp(WriteStartPosition);
		WriteLE(&BlockSize, *StreamPtr);
		//Go back to where we left off writing so any subsequent data follows this data block
		StreamPtr->seekp(WriteEndPosition);
	}

	ScopedDataBlockRead::ScopedDataBlockRead(std::istream& Stream)
	: StreamPtr(&Stream)
	{
		BlockSizeType BlockSize = 0;
		ReadLE(&BlockSize, Stream);
		//We have already read the size marker, so the read end position is the current location plus the block size.
		ReadEndPosition = (Stream.tellg() + std::streamoff{BlockSize});
	}

	ScopedDataBlockRead::~ScopedDataBlockRead() {
		//Seek ahead to the end position (we may already be there if the entire data block was read)
		StreamPtr->seekg(ReadEndPosition);
	}

	int64_t ScopedDataBlockRead::GetRemainingSize() const {
		return int64_t(ReadEndPosition - StreamPtr->tellg());
	}

	void ScopedDataBlockRead::SkipNestedBlock() const {
		const std::streampos CurrentPosition = StreamPtr->tellg();
		BlockSizeType NestedBlockSize = 0;
		ReadLE(&NestedBlockSize, *StreamPtr);
		StreamPtr->seekg(std::min(CurrentPosition + std::streamoff{NestedBlockSize}, ReadEndPosition));
	}

	void ResetStream( std::stringstream& Stream ) {
		Stream.str("");
		Stream.clear();
	}
}
