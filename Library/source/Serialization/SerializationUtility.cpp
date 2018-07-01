#include "Serialization/SerializationUtility.h"
#include "Serialization/ByteUtility.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Serialization {
	using BLOCK_SIZE_TYPE = uint32_t;

	std::streampos StartDataBlockWrite( std::ostream& Stream ) {
		std::streampos const StartPosition = Stream.tellp();
		//Write zeros to the stream to reserve a space where the block size will eventually be written
		BLOCK_SIZE_TYPE const Zero = 0;
		WriteLE<BLOCK_SIZE_TYPE>( &Zero, Stream ); //Endianness doesn't matter, but kept this way for parity.
		//Return where the block starts so we can seek back here when finishing
		return StartPosition;
	}

	void FinishDataBlockWrite( std::ostream& Stream, std::streampos StartPosition ) {
		std::streampos const EndPosition = Stream.tellp();
		BLOCK_SIZE_TYPE const BlockDataSize = EndPosition - StartPosition - sizeof( BLOCK_SIZE_TYPE );
		//Reverse back to the start of the block and write the size to the reserved bytes
		Stream.seekp( StartPosition );
		WriteLE<BLOCK_SIZE_TYPE>( &BlockDataSize, Stream );
		//Go back to the end of the block so that further data can be written to the stream
		Stream.seekp( EndPosition );
	}

	std::streampos ReadDataBlockEndPosition( std::istream& Stream ) {
		BLOCK_SIZE_TYPE NumBytes = 0;
		ReadLE<BLOCK_SIZE_TYPE>( &NumBytes, Stream );
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
