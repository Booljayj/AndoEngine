#include <set>
#include "Serialization/StructSerializer.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/VariableInfo.h"
#include "Serialization/ByteUtility.h"
#include "Serialization/SerializationUtility.h"

namespace Serialization {
	StructSerializer::StructSerializer( Reflection::StructTypeInfo const* InType )
	: Type( InType )
	{}

	void StructSerializer::SerializeBinary( void const* Data, std::ostream& Stream ) const {
		std::streampos const StartPosition = StartDataBlockWrite( Stream );
		void const* DefaultData = Type->Default;

		//Write an identifier and a data block for each non-default variable in the struct
		Reflection::StructTypeInfo const* CurrentType = Type;
		while( CurrentType ) {
			for( Reflection::VariableInfo const* VariableInfo : CurrentType->Member.Variables ) {
				Reflection::TypeInfo const* Type = VariableInfo->Type;
				if( Type->Serializer ) {
					//Get a pointer to the value we want to serialize, and a pointer to the default version of that value
					void const* VariablePointer = VariableInfo->GetImmutableValuePointer( Data );
					void const* DefaultVariablePointer = VariableInfo->GetImmutableValuePointer( DefaultData );

					//Compare the variable to the default. If the value is the same as the default, then we don't need to write anything
					if( !AreValuesEqual( Type, VariablePointer, DefaultVariablePointer ) )
					{
						WriteVariableIdentifier( VariableInfo, Stream );
						Type->Serializer->SerializeBinary( VariablePointer, Stream );
					}
				}
			}
			CurrentType = CurrentType->BaseType;
		}

		FinishDataBlockWrite( Stream, StartPosition );
	}

	bool StructSerializer::DeserializeBinary( void* Data, std::istream& Stream ) const {
		std::streampos const EndPosition = ReadDataBlockEndPosition( Stream );

		while( Stream.good() && CanReadNextVariableHeader( Stream, EndPosition ) ) {
			//Get the next variable to deserialize
			Reflection::VariableInfo const* Variable = ReadVariableIdentifier( Stream );
			if( !Stream.good() ) return false; //Make sure the read was successful

			//If the struct has a variable with this ID, and it can be serialized, attempt to deserialize it.
			if( Variable && Variable->Type->Serializer ) {
				void* VariablePointer = Variable->GetMutableValuePointer( Data );
				bool const bSuccess = Variable->Type->Serializer->DeserializeBinary( VariablePointer, Stream );
				if( !bSuccess ) return false; //@todo Report an error just for this variable instead of returning false

			} else {
				HandleUnknownDataBlock( Stream );
			}

			if( Stream.tellg() > EndPosition ) return false;
		}

		//Seek to the end of the data block in case we aren't there already
		Stream.seekg( EndPosition );
		return true;
	}

	void StructSerializer::SerializeText( void const* Data, std::ostringstream& Stream ) const {}
	bool StructSerializer::DeserializeText( void* Data, std::istringstream& Stream ) const { return false; }

	void StructSerializer::WriteVariableIdentifier( Reflection::VariableInfo const* VariableInfo, std::ostream& Stream ) const {
		WriteLE( &(VariableInfo->NameHash), Stream );
	}

	void StructSerializer::WriteVariableStream( std::istream& VariableStream, std::ostream& Stream ) const {
		Stream << VariableStream.rdbuf();
	}

	bool StructSerializer::CanReadNextVariableHeader( std::istream& Stream, std::streampos const& EndPosition ) const {
		return CanReadBytesFromStream( sizeof( Reflection::VariableInfo::HASH_T ) + sizeof( DataBlock::BLOCK_SIZE_T ), Stream, EndPosition );
	}

	Reflection::VariableInfo const* StructSerializer::ReadVariableIdentifier( std::istream& Stream ) const {
		Reflection::VariableInfo::HASH_T NameHash = 0;
		ReadLE( &NameHash, Stream );

		//Walk up the chain of base classes, searching for a variable with the correct name hash.
		Reflection::StructTypeInfo const* CurrentType = Type;
		Reflection::VariableInfo const* FoundVariable = nullptr;
		while( CurrentType && !FoundVariable ) {
			FoundVariable = CurrentType->FindMemberVariableInfo( NameHash );
			CurrentType = CurrentType->BaseType;
		}
		return FoundVariable;
	}

	void StructSerializer::HandleUnknownDataBlock( std::istream& Stream ) const {
		std::streampos const EndPosition = ReadDataBlockEndPosition( Stream );
		Stream.seekg( EndPosition );
	}
}
