#include "Serialization/StructSerializer.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/VariableInfo.h"
#include "Serialization/ByteUtility.h"
#include "Serialization/SerializationUtility.h"

namespace Serialization {
	StructSerializer::StructSerializer( Reflection::StructTypeInfo const* InType )
	: Type( InType )
	{}

	void StructSerializer::SerializeBinary( void const* Data, std::ostream& Stream )
	{
		CacheVariables();
		//Stream used to record variable data before actually writing it to the input stream, so that the data block size can be deduced.
		std::stringstream VariableDataStream;
		void const* DefaultData = Type->Default.get();

		for( Reflection::MemberVariableInfo const* Variable : CachedMemberVariables ) {
			Reflection::TypeInfo* Type = Variable->Type;
			Type->Load();

			if( Type->Serializer ) {
				//Get a pointer to the value we want to serialize, and a pointer to the default version of that value
				void const* VariableDataPointer = Variable->GetValuePointer( Data );
				void const* DefaultDataPointer = Variable->GetValuePointer( DefaultData );

				//Compare the variable to the default (if a compare is available). If the value is the same as the default, then we don't need to write anything
				if( !AreValuesEqual( Type, VariableDataPointer, DefaultDataPointer ) )
				{
					//Write all the variable's data to the variable data stream using the serializer
					ResetStream( VariableDataStream );
					Type->Serializer->SerializeBinary( VariableDataPointer, VariableDataStream );

					//Find the metadata that will be preceed the data block
					uint16_t const VariableNameHash = Variable->NameHash;
					uint32_t const VariableDataSize = GetDataBlockSize( VariableDataStream );

					if( IsDataBlockSizeValid( VariableDataSize ) ) {
						LittleEndianByteSerializer<sizeof( VariableNameHash )>::Write( &VariableNameHash, Stream );
						LittleEndianByteSerializer<sizeof( VariableDataSize )>::Write( &VariableDataSize, Stream );
						Stream << VariableDataStream.rdbuf();
					}
				}
			}
		}
	}

	bool StructSerializer::DeserializeBinary( void* Data, std::istream& Stream, uint32_t NumBytes )
	{
		std::streampos CurrentPosition = Stream.tellg();
		std::streampos const EndPosition = CurrentPosition + std::streamoff{ NumBytes };

		while( Stream.good() && ( CurrentPosition < EndPosition ) ) {
			//Read the next variable ID and data block size from the stream
			uint16_t VariableNameHash = 0;
			uint32_t VariableDataSize = 0;
			LittleEndianByteSerializer<sizeof( VariableNameHash )>::Read( &VariableNameHash, Stream );
			LittleEndianByteSerializer<sizeof( VariableDataSize )>::Read( &VariableDataSize, Stream );

			//@todo This is a temporary fix because of how streams do not report EOF until you try to read past-the-end.
			//Break out of the loop when we try to read a data block header but fail
			if( !Stream.good() ) break;

			//Determine where the data block for this variable ends, using the current position and the size we just read
			std::streampos const VariableDataEndPosition = Stream.tellg() + std::streamoff{ VariableDataSize };
			//If the block size is invalid, the stream is corrupt, and we should stop now.
			if( VariableDataEndPosition > EndPosition ) return false;

			//If the struct has a variable with this ID, and it can be serialized, attempt to deserialize it.
			if( Reflection::MemberVariableInfo const* Variable = FindVariableWithNameHash( VariableNameHash ) ) {
				Reflection::TypeInfo* Type = Variable->Type;
				Type->Load();
				if( Type->Serializer ) {
					void* VariableDataPointer = Variable->GetValuePointer( Data );
					bool const bSuccess = Variable->Type->Serializer->DeserializeBinary( VariableDataPointer, Stream, VariableDataSize );
					if( !bSuccess ) return false; //@todo Report an error just for this variable instead of returning false.
				}
			}

			//Move ahead in the stream to where the next variable should be (should already be there if everything went well)
			Stream.seekg( VariableDataEndPosition );
			CurrentPosition = VariableDataEndPosition;
		}

		if( NumBytes == UINT32_MAX || Stream.tellg() == EndPosition ) return true;
		else return false;
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

	Reflection::MemberVariableInfo const* StructSerializer::FindVariableWithNameHash( uint16_t NameHash ) const {
		auto const FoundVariable = std::find_if(
			CachedMemberVariables.begin(), CachedMemberVariables.end(),
			[=]( auto const* A ){ return A->NameHash == NameHash; }
		);
		return FoundVariable != CachedMemberVariables.end() ? *FoundVariable : nullptr;
	}
}
