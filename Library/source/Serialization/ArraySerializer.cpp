#include "Serialization/ArraySerializer.h"
#include "Reflection/ArrayTypeInfo.h"
#include "Reflection/Components/VariableInfo.h"
#include "Serialization/ByteUtility.h"
#include "Serialization/SerializationUtility.h"

namespace Serialization {
	ArraySerializer::ArraySerializer( Reflection::ArrayTypeInfo const* InType )
	: Type( InType )
	{}

	void ArraySerializer::SerializeBinary( void const* Data, std::ostream& Stream )
	{
		std::streampos const StartPosition = StartDataBlockWrite( Stream );
		std::vector<void const*> Elements;

		//Write the size of the array to the stream (handles cases where array size changes)
		WriteArrayCount( Stream, Data );

		//Get an array of pointers to all the elements
		Type->GetElements( Data, Elements );

		//Write a data block for each element to the stream
		for( uint32_t Index = 0; Index < Elements.size(); ++Index ) {
			Type->ElementType->Serializer->SerializeBinary( Elements[Index], Stream );
		}

		FinishDataBlockWrite( Stream, StartPosition );
	}

	bool ArraySerializer::DeserializeBinary( void* Data, std::istream& Stream )
	{
		std::streampos const EndPosition = ReadDataBlockEndPosition( Stream );

		//Get the serialized size of the array from the data block
		uint32_t const SerializedArraySize = ReadArrayCount( Stream );
		//Set the array size to match what we just read, if possible
		if( !Type->IsFixedSize ) Type->Resize( Data, SerializedArraySize );

		//Get an array of pointers to all the elements
		std::vector<void*> Elements;
		Type->GetElements( Data, Elements );

		//Read a data block for each element from the stream
		bool bArrayReadSuccessful = true;
		for( uint32_t Index = 0; Index < Elements.size(); ++Index ) {
			if( Index < SerializedArraySize && CanReadNextElementHeader( Stream, EndPosition ) ) {
				bool const bElementReadSuccessful = Type->ElementType->Serializer->DeserializeBinary( Elements[Index], Stream );
				bArrayReadSuccessful &= bElementReadSuccessful;
			}
		}

		Stream.seekg( EndPosition );
		return Stream.good();
	}

	void ArraySerializer::SerializeText( void const* Data, std::ostringstream& Stream ) {}
	bool ArraySerializer::DeserializeText( void* Data, std::istringstream& Stream ) { return false; }

	bool ArraySerializer::CanReadNextElementHeader( std::istream& Stream, std::streampos const& EndPosition ) const {
		return CanReadBytesFromStream( sizeof( uint32_t ), Stream, EndPosition );
	}

	void ArraySerializer::WriteArrayCount( std::ostream& Stream, void const* Data ) const {
		//@todo Impose a limit on the size of serialized arrays, arrays that are too large should just write nothing
		uint32_t ArraySize = Type->GetCount( Data );
		WriteLE( &ArraySize, Stream );
	}

	uint32_t ArraySerializer::ReadArrayCount( std::istream& Stream ) const {
		uint32_t ArraySize = 0;
		ReadLE( &ArraySize, Stream );
		return ArraySize;
	}
}
