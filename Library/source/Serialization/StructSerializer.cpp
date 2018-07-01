#include "Serialization/StructSerializer.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/VariableInfo.h"
#include "Serialization/ByteUtility.h"
#include "Serialization/SerializationUtility.h"

namespace Serialization {
	StructSerializer::StructSerializer( Reflection::StructTypeInfo const* InType )
	: Type( InType )
	{}

	void StructSerializer::SerializeBinary( void const* Data, std::ostream& Stream ) {
		//Set up the list of variables that this type contains
		CacheVariables();

		std::streampos const StartPosition = StartDataBlockWrite( Stream );
		void const* DefaultData = Type->Default.get();

		//Write an identifier and a data block for each non-default variable in the struct
		for( Reflection::MemberVariableInfo const* VariableInfo : CachedMemberVariables ) {
			Reflection::TypeInfo const* Type = VariableInfo->Type;
			if( Type->Serializer ) {
				//Get a pointer to the value we want to serialize, and a pointer to the default version of that value
				void const* VariablePointer = VariableInfo->GetValuePointer( Data );
				void const* DefaultVariablePointer = VariableInfo->GetValuePointer( DefaultData );

				//Compare the variable to the default. If the value is the same as the default, then we don't need to write anything
				if( !AreValuesEqual( Type, VariablePointer, DefaultVariablePointer ) )
				{
					WriteVariableIdentifier( VariableInfo, Stream );
					Type->Serializer->SerializeBinary( VariablePointer, Stream );
				}
			}
		}

		FinishDataBlockWrite( Stream, StartPosition );
	}

	bool StructSerializer::DeserializeBinary( void* Data, std::istream& Stream ) {
		std::streampos const EndPosition = ReadDataBlockEndPosition( Stream );

		while( Stream.good() && CanReadNextVariableHeader( Stream, EndPosition ) ) {
			//Get the next variable to deserialize
			Reflection::MemberVariableInfo const* Variable = ReadVariableIdentifier( Stream );
			if( !Stream.good() ) return false; //Make sure the read was successful

			//If the struct has a variable with this ID, and it can be serialized, attempt to deserialize it.
			if( Variable && Variable->Type->Serializer ) {
				void* VariablePointer = Variable->GetValuePointer( Data );
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

	void StructSerializer::SerializeText( void const* Data, std::ostringstream& Stream ) {}
	bool StructSerializer::DeserializeText( void* Data, std::istringstream& Stream ) { return false; }

	void StructSerializer::CacheVariables() {
		if( CachedMemberVariables.size() == 0 ) {
			Type->GetMemberVariablesRecursive( CachedMemberVariables );
			//Remove invalid variables
			auto const NewEnd = std::remove_if(
				CachedMemberVariables.begin(), CachedMemberVariables.end(),
				[]( auto const* A ){ return !A || !A->Type; }
			);
			CachedMemberVariables.erase( NewEnd, CachedMemberVariables.end() );
			//Sort the remaining variables by NameHash for quick lookups
			std::sort(
				CachedMemberVariables.begin(), CachedMemberVariables.end(),
				[]( auto const* A, auto const* B ){ return A->NameHash < B->NameHash; }
			);
		}
	}

	void StructSerializer::WriteVariableIdentifier( Reflection::MemberVariableInfo const* VariableInfo, std::ostream& Stream ) const {
		WriteLE<uint16_t>( &(VariableInfo->NameHash), Stream );
	}

	void StructSerializer::WriteVariableStream( std::istream& VariableStream, std::ostream& Stream ) const {
		Stream << VariableStream.rdbuf();
	}

	bool StructSerializer::CanReadNextVariableHeader( std::istream& Stream, std::streampos const& EndPosition ) const {
		return CanReadBytesFromStream( sizeof( uint16_t ) + sizeof( uint32_t ), Stream, EndPosition );
	}

	Reflection::MemberVariableInfo const* StructSerializer::ReadVariableIdentifier( std::istream& Stream ) const {
		uint16_t NameHash = 0;
		ReadLE<uint16_t>( &NameHash, Stream );

		auto const FoundVariable = std::find_if(
			CachedMemberVariables.begin(), CachedMemberVariables.end(),
			[=]( auto const* A ){ return A->NameHash == NameHash; }
		);
		return FoundVariable != CachedMemberVariables.end() ? *FoundVariable : nullptr;
	}

	void StructSerializer::HandleUnknownDataBlock( std::istream& Stream ) const {
		std::streampos const EndPosition = ReadDataBlockEndPosition( Stream );
		Stream.seekg( EndPosition );
	}
}
