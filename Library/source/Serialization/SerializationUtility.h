#pragma once
#include "Serialization/Serializer.h"
#include <ostream>
#include <sstream>

namespace Reflection {
	struct TypeInfo;
	struct MemberVariableInfo;
}

namespace Serialization {
	//@todo Make this a contextual function so that it can print an error if there is too much data in the source stream to form a data block.
	/** Write a binary data block composed of the data in the source stream to the target stream */
	void WriteDataBlock( std::stringstream& SourceStream, std::ostream& TargetStream );

	/** Read the end of the data block from the stream */
	std::streampos ReadDataBlockEndPosition( std::istream& Stream );

	/** True if the amount of bytes can be read from the stream without going past the end position */
	bool CanReadBytesFromStream( uint32_t NumBytes, std::istream& Stream, std::streampos const& EndPosition );

	/** Reset the stringstream contents */
	void ResetStream( std::stringstream& Stream );
	/** True if the data in the two pointers are equal */
	bool AreValuesEqual( Reflection::TypeInfo const* Info, void const* ValueA, void const* ValueB );
}
