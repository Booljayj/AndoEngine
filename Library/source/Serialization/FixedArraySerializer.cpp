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
		//Get the size of the array and write it to the stream
		uint32_t const ArraySize = Type->Count;
		LittleEndianByteSerializer<sizeof( uint32_t )>::Write( &ArraySize, Stream );
		//Get an array of pointers to all the elements
		std::vector<void const*> Elements;
		Type->GetElements( Data, Elements );
		//Write a data block for each element to the stream
		std::stringstream ElementDataStream;
		for( int32_t Index = 0; Index < ArraySize; ++Index ) {
			ResetStream( ElementDataStream );
			WriteBinaryDataBlock( *Type->ElementType->Serializer, Elements[Index], Stream, ElementDataStream );
		}
	}

	bool FixedArraySerializer::DeserializeBinary( void* Data, std::istream& Stream, uint32_t NumBytes )
	{
		//Get the serialized size of the array from the data block and the actual size of the array
		uint32_t SerializedArraySize;
		LittleEndianByteSerializer<sizeof( SerializedArraySize )>::Read( &SerializedArraySize, Stream );
		uint32_t const ActualArraySize = Type->Count;
		//Get an array of pointers to all the elements
		std::vector<void*> Elements;
		Type->GetElements( Data, Elements );
		//Read a data block for each element from the stream
		for( int32_t Index = 0; Index < ActualArraySize; ++Index ) {
			uint32_t ElementSize;
			LittleEndianByteSerializer<sizeof( ElementSize )>::Read( &ElementSize, Stream );
			Type->ElementType->Serializer->DeserializeBinary( Elements[Index], Stream, ElementSize );
		}
		return Stream.good();
	}

	void FixedArraySerializer::SerializeText( void const* Data, std::ostringstream& Stream ) {}
	bool FixedArraySerializer::DeserializeText( void* Data, std::istringstream& Stream ) { return false; }
}
