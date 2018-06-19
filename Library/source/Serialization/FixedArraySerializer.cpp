#include "Serialization/FixedArraySerializer.h"
#include "Reflection/FixedArrayTypeInfo.h"
#include "Reflection/Components/VariableInfo.h"
#include "Serialization/ByteUtility.h"
#include "Serialization/SerializationUtility.h"

namespace Serialization {
	FixedArraySerializer::FixedArraySerializer( Reflection::FixedArrayTypeInfo const* InType )
	: Type( InType )
	{}

	void FixedArraySerializer::SerializeBinary( void const* Data, std::ostream& Stream )
	{
		std::stringstream ArrayDataStream;
		std::stringstream ElementDataStream;
		std::vector<void const*> Elements;

		//Write the size of the array to the stream (handles cases where array size changes)
		WriteArraySize( ArrayDataStream, Data );

		//Get an array of pointers to all the elements
		Type->GetElements( Data, Elements );

		//Write a data block for each element to the stream
		for( uint32_t Index = 0; Index < Type->Count; ++Index ) {
			ResetStream( ElementDataStream );
			Type->ElementType->Serializer->SerializeBinary( Elements[Index], ElementDataStream );
			WriteDataBlock( ElementDataStream, ArrayDataStream );
		}

		WriteDataBlock( ArrayDataStream, Stream );
	}

	bool FixedArraySerializer::DeserializeBinary( void* Data, std::istream& Stream )
	{
		std::streampos const EndPosition = ReadDataBlockEndPosition( Stream );

		//Get the serialized size of the array from the data block
		uint32_t const SerializedArraySize = ReadArraySize( Stream );

		//Get an array of pointers to all the elements
		std::vector<void*> Elements;
		Type->GetElements( Data, Elements );

		//Read a data block for each element from the stream
		bool bArrayReadSuccessful = true;
		for( uint32_t Index = 0; Index < Type->Count; ++Index ) {
			if( Index < SerializedArraySize && CanReadNextElementHeader( Stream, EndPosition ) ) {
				bool const bElementReadSuccessful = Type->ElementType->Serializer->DeserializeBinary( Elements[Index], Stream );
				bArrayReadSuccessful &= bElementReadSuccessful;
			}
		}

		Stream.seekg( EndPosition );
		return Stream.good();
	}

	void FixedArraySerializer::SerializeText( void const* Data, std::ostringstream& Stream ) {}
	bool FixedArraySerializer::DeserializeText( void* Data, std::istringstream& Stream ) { return false; }

	bool FixedArraySerializer::CanReadNextElementHeader( std::istream& Stream, std::streampos const& EndPosition ) const {
		return CanReadBytesFromStream( sizeof( uint32_t ), Stream, EndPosition );
	}

	void FixedArraySerializer::WriteArraySize( std::ostream& Stream, void const* Data ) const {
		WriteLE<uint32_t>( &(Type->Count), Stream );
	}

	uint32_t FixedArraySerializer::ReadArraySize( std::istream& Stream ) const {
		uint32_t ArraySize = 0;
		ReadLE<uint32_t>( &ArraySize, Stream );
		return ArraySize;
	}
}
