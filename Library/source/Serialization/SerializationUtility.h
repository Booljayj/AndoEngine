#pragma once
#include "Serialization/Serializer.h"
#include <ostream>
#include <sstream>

namespace Reflection {
	struct TypeInfo;
	struct MemberVariableInfo;
}

namespace Serialization {
	namespace DataBlock {
		/** Type used to store the size of a data block */
		using BLOCK_SIZE_T = uint32_t;
	}

	/** Start a data block in the stream. Returns the position where the block starts */
	std::streampos StartDataBlockWrite( std::ostream& Stream );
	/** Finish a data block in the stream */
	void FinishDataBlockWrite( std::ostream& Stream, std::streampos StartPosition );

	/** Read the end of the data block from the stream */
	std::streampos ReadDataBlockEndPosition( std::istream& Stream );

	/** True if the amount of bytes can be read from the stream without going past the end position */
	bool CanReadBytesFromStream( uint32_t NumBytes, std::istream& Stream, std::streampos const& EndPosition );

	/** Reset the stringstream contents */
	void ResetStream( std::stringstream& Stream );
	/** True if the data in the two pointers are equal */
	bool AreValuesEqual( Reflection::TypeInfo const* Info, void const* ValueA, void const* ValueB );
}
