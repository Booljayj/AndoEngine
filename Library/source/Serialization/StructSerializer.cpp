#include "Serialization/StructSerializer.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Serialization {
	StructSerializer::StructSerializer( Reflection::StructTypeInfo const* InType )
	: Type( InType )
	{}

	void StructSerializer::Load()
	{
		if( CachedMemberVariables.size() == 0 ) {
			Type->GetMemberVariablesRecursive( CachedMemberVariables );
			std::sort(
				CachedMemberVariables.begin(), CachedMemberVariables.end(),
				[]( auto const* A, auto const* B){ return A->NameHash < B->NameHash; }
			);
		}
	}

	void StructSerializer::SerializeBinary( void const* Data, std::ostream& Stream ) const
	{
		//Stream used to record variable data before actually writing it to the input stream, so that the data block size can be deduced.
		std::stringstream VariableDataStream;

		void const* DefaultData = Type->Default.get();
		for( Reflection::MemberVariableInfo const* Variable : CachedMemberVariables ) {
			if( Variable && Variable->Type && Variable->Type->Serializer ) {
				uint16_t const VariableNameHash = Variable->NameHash;

				//Get a pointer to the value we want to serialize, and a pointer to the default version of that value
				void const* VariableDataPointer = Variable->GetValuePointer( Data );
				void const* DefaultDataPointer = Variable->GetValuePointer( DefaultData );

				//Compare the variable to the default (if a compare is available). If the value is the same as the default, then we don't need to write anything
				if( Variable->Type->Compare && Variable->Type->Compare( Variable->Type, VariableDataPointer, DefaultDataPointer ) == 0 ) {
					continue;
				}

				//Reset the variable data stream
				VariableDataStream.str( "" );
				VariableDataStream.clear();
				//Write all the variable's data to the variable data stream
				Variable->Type->Serializer->SerializeBinary( VariableDataPointer, VariableDataStream );

				//Find the amount of data that was written to the variable data stream
				size_t const FullVariableDataSize = VariableDataStream.tellp();
				uint32_t const VariableDataSize = FullVariableDataSize;
				//Make sure the size of the data does not exceed the size of a uint32_t
				if( FullVariableDataSize != VariableDataSize ) {
					continue;
				}

				if( VariableDataSize > 0 ) {
					Stream << VariableNameHash << VariableDataSize;
					Stream << VariableDataStream.rdbuf();
				}
			}
		}
	}

	bool StructSerializer::DeserializeBinary( void* Data, std::istream& Stream, uint32_t NumBytes ) const
	{
		std::streampos CurrentPosition = Stream.tellg();
		std::streampos const EndPosition = CurrentPosition + std::streamoff{ NumBytes };

		while( Stream.good() && ( CurrentPosition < EndPosition ) ) {
			//Read the next variable ID and data block size from the stream
			uint16_t VariableNameHash = 0;
			uint32_t VariableDataSize = 0;
			Stream >> VariableNameHash >> VariableDataSize;

			//Determine where the data block for this variable ends, using the current position and the size we just read
			std::streampos VariableDataEndPosition = Stream.tellg() + std::streamoff{ VariableDataSize };

			//Ensure that the variable data block is valid
			if( !Stream.good() || VariableDataEndPosition > EndPosition ) return false;

			//Find the variable using its ID
			//@todo Use a binary search here for better performance, since the cached array of variables should be in order by NameHash
			auto const FoundVariable = std::find_if(
				CachedMemberVariables.begin(), CachedMemberVariables.end(),
				[=]( auto const* A ){ return A->NameHash == VariableNameHash; }
			);
			Reflection::MemberVariableInfo const* Variable = FoundVariable != CachedMemberVariables.end() ? *FoundVariable : nullptr;

			//If the struct has a variable with this ID, and it can be serialized, attempt to deserialize it.
			if( Variable && Variable->Type && Variable->Type->Serializer ) {
				void* VariableDataPointer = Variable->GetValuePointer( Data );
				bool const bSuccess = Variable->Type->Serializer->DeserializeBinary( VariableDataPointer, Stream, VariableDataSize );
				if( !bSuccess ) return false; //@todo Report an error just for this variable instead of returning false.
			}

			//Move ahead in the stream to where the next variable should be (should already be there if everything went well)
			Stream.seekg( VariableDataEndPosition );
			CurrentPosition = VariableDataEndPosition;
		}

		if( Stream.tellg() == EndPosition ) return true;
		else return false;
	}

	void StructSerializer::SerializeText( void const* Data, std::ostringstream& Stream ) const {}
	bool StructSerializer::DeserializeText( void* Data, std::istringstream& Stream ) const { return false; }
}
