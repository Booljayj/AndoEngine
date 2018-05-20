#pragma once
#include <ostream>
#include <sstream>

namespace Reflection {
	struct TypeInfo;
}

namespace Serialization {
	/** Get the size of the data block encoded in the stream as a uint32_t */
	uint32_t GetDataBlockSize( std::ostream& Stream );
	/** True if the data block size does not exceed limits */
	bool IsDataBlockSizeValid( uint32_t Size );
	/** Reset the stringstream contents */
	void ResetStream( std::stringstream& Stream );
	/** True if the data in the two pointers are equal */
	bool AreValuesEqual( Reflection::TypeInfo* Info, void const* ValueA, void const* ValueB );
}
