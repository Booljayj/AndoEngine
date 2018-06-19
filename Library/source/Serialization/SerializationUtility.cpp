#include "Serialization/SerializationUtility.h"
#include "Serialization/ByteUtility.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Serialization {
	void WriteDataBlock( std::stringstream& SourceStream, std::ostream& TargetStream ) {
		size_t SourceStreamSize = SourceStream.tellp();

		if( SourceStreamSize > UINT32_MAX ) {
			//There is too much data in the source stream to form a data block, so write an empty block
			char Zero[sizeof(uint32_t)] = { 0 };
			TargetStream.write( Zero, sizeof( uint32_t ) );

		} else {
			WriteLE<uint32_t>( &SourceStreamSize, TargetStream );
			TargetStream << SourceStream.rdbuf();
		}
	}

	std::streampos ReadDataBlockEndPosition( std::istream& Stream ) {
		uint32_t NumBytes = 0;
		ReadLE<uint32_t>( &NumBytes, Stream );
		return ( Stream.tellg() + std::streamoff{ NumBytes } );
	}

	bool CanReadBytesFromStream( uint32_t NumBytes, std::istream& Stream, std::streampos const& EndPosition ) {
		return ( Stream.tellg() + std::streamoff{ NumBytes } ) <= EndPosition;
	}

	void ResetStream( std::stringstream& Stream ) {
		Stream.str( "" );
		Stream.clear();
	}

	bool AreValuesEqual( Reflection::TypeInfo const* Info, void const* ValueA, void const* ValueB ) {
		if( ValueA && ValueB ) {
			return Info->Compare( ValueA, ValueB ) == 0;
		}
		return false;
	}
}
