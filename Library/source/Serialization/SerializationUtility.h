#pragma once
#include "Serialization/Serializer.h"
#include <ostream>
#include <sstream>

namespace Reflection {
	struct TypeInfo;
}

namespace Serialization {
	// Binary data blocks consist of a size followed by a package of serialized data. The serialized data may recursively contain other data blocks.
	/** Write a binary data block */
	void WriteBinaryDataBlock( ISerializer& Serializer, void const* Value, std::ostream& Stream, std::stringstream& ScratchStream );
	/** Read a binary data block */
	//void ReadBinaryDataBlock( Reflection::TypeInfo const& Info, void* Value, std::istream& Stream );

	/** Get the size of the data block encoded in the stream as a uint32_t */
	uint32_t GetDataBlockSize( std::ostream& Stream );
	/** True if the data block size does not exceed limits */
	bool IsDataBlockSizeValid( uint32_t Size );
	/** Reset the stringstream contents */
	void ResetStream( std::stringstream& Stream );
	/** True if the data in the two pointers are equal */
	bool AreValuesEqual( Reflection::TypeInfo const* Info, void const* ValueA, void const* ValueB );
}
