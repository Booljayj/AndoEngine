#include "Serialization/SerializationUtility.h"
#include "Reflection/TypeInfo.h"

namespace Serialization {
	uint32_t GetDataBlockSize( std::ostream& Stream ) {
		size_t StreamSize = Stream.tellp();
		if( StreamSize > UINT32_MAX ) {
			return UINT32_MAX;
		} else {
			return StreamSize;
		}
	}

	bool IsDataBlockSizeValid( uint32_t Size ) {
		return Size > 0 && Size < UINT32_MAX;
	}

	void ResetStream( std::stringstream& Stream ) {
		Stream.str( "" );
		Stream.clear();
	}

	bool AreValuesEqual( Reflection::TypeInfo* Info, void const* ValueA, void const* ValueB ) {
		if( ValueA && ValueB && Info->Compare ) {
			return Info->Compare( Info, ValueA, ValueB ) == 0;
		}
		return false;
	}
}
