#include <iomanip>
#include "Reflection/TypeInfo.h"
#include "Reflection/StructTypeInfo.h"
#include "Engine/StringID.h"

namespace Reflection {
	std::deque<TypeInfo const*> TypeInfo::GlobalTypeCollection{};

	void TypeInfo::Print( TypeInfo const* Info, std::ostream& Stream ) {
		if( !Info) {
			Stream << "{{INVALID TYPE}}";
			return;
		}

		//Print attributes
		if( Info->Serializer ) {
			Stream << "[Serializable]\n";
		}
		//Print type and subtype information
		if( StructTypeInfo const* StructInfo = Info->As<StructTypeInfo>() ) {
			//Print struct header
			Stream << "struct " << StructInfo->Name;
			if( StructInfo->BaseType ) {
				Stream << " : " << StructInfo->BaseType->Name;
			}
			Stream << " {\n";

			//Print constants
			if( StructInfo->StaticConstants.size() > 0 || StructInfo->MemberConstants.size() > 0 ) {
				for( auto const& StaticConstant : StructInfo->StaticConstants ) {
					Stream << "\t" << StaticConstant->Name << " : " << StaticConstant->Type->Name << " const static\n";
				}
				for( auto const& MemberConstant : StructInfo->MemberConstants ) {
					Stream << "\t" << MemberConstant->Name << " : " << MemberConstant->Type->Name << " const\n";
				}
				Stream << "\n";
			}
			//Print variables
			if( StructInfo->StaticVariables.size() > 0 || StructInfo->MemberVariables.size() > 0 ) {
				for( auto const& StaticVariable : StructInfo->StaticVariables ) {
					Stream << "\t" << StaticVariable->Name << " : " << StaticVariable->Type->Name << " static\n";
				}
				for( auto const& MemberVariable : StructInfo->MemberVariables ) {
					Stream << "\t" << MemberVariable->Name << " : " << MemberVariable->Type->Name << "\n";
				}
				Stream << "\n";
			}
			Stream << "}\n";

		} else {
			Stream << Info->Name;
		}
	}

	void TypeInfo::PrintAll( std::ostream& Stream ) {
		for( Reflection::TypeInfo const* Info : Reflection::TypeInfo::GlobalTypeCollection ) {
			if( Info ) {
				Stream << std::hex << std::right << std::setw( 8 ) << std::setfill( '0' ) << Info->NameHash
					<< std::dec << std::setw( 0 );
				Stream << " " << Info->Name << " (" << Info->Size << ")" << std::endl;
			} else {
				Stream << "{{INVALID TYPE}}" << std::endl;
			}
		}
	}

	TypeInfo const* TypeInfo::FindTypeByNameHash( HASH_T NameHash ) {
		for( TypeInfo const* Info : GlobalTypeCollection ) {
			if( Info && Info->NameHash == NameHash ) {
				return Info;
			}
		}
		return nullptr;
	}
	TypeInfo const* TypeInfo::FindTypeByName( std::string_view Name ) {
		for( TypeInfo const* Info : GlobalTypeCollection ) {
			if( Info && Info->Name == Name ) {
				return Info;
			}
		}
		return nullptr;
	}

	TypeInfo::TypeInfo( ETypeClassification InClassification, std::string_view InName, size_t InSize, size_t InAlignment, std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer )
		: Classification( InClassification )
		, Name( InName )
		, NameHash( static_cast<HASH_T>( id( InName ) ) )
		, Size( InSize )
		, Alignment( InAlignment )
		, Description( InDescription )
		, Flags( InFlags )
		, Serializer( InSerializer )
	{
		GlobalTypeCollection.push_back( this );
	}
	TypeInfo::TypeInfo( ETypeClassification InClassification, std::string_view InName, size_t InSize, size_t InAlignment )
		: TypeInfo( InClassification, InName, InSize, InAlignment, "", (FTypeFlags)0, nullptr )
	{}

	int8_t TypeInfo::Compare( void const* A, void const* B ) const {
		return memcmp( A, B, Size );
	}
}
