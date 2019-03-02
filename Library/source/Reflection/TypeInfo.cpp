#include <iomanip>
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeUtility.h"
#include "Reflection/StructTypeInfo.h"
#include "Engine/StringID.h"

namespace Reflection {
	/** The global list of all TypeInfo objects that have been created */
	std::deque<TypeInfo const*> GlobalTypeCollection{};

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
			Stream << "struct " << StructInfo->Definition.GetMangledName();
			if( StructInfo->BaseType ) {
				Stream << " : " << StructInfo->BaseType->Definition.GetMangledName();
			}
			Stream << " {\n";

			//Print constants
			if( StructInfo->Static.Constants.Size() > 0 || StructInfo->Member.Constants.Size() > 0 ) {
				for( auto const& StaticConstant : StructInfo->Static.Constants ) {
					Stream << "\t" << StaticConstant->Name << " : " << StaticConstant->Type->Definition.GetMangledName() << " const static\n";
				}
				for( auto const& MemberConstant : StructInfo->Member.Constants ) {
					Stream << "\t" << MemberConstant->Name << " : " << MemberConstant->Type->Definition.GetMangledName() << " const\n";
				}
				Stream << "\n";
			}
			//Print variables
			if( StructInfo->Static.Variables.Size() > 0 || StructInfo->Member.Variables.Size() > 0 ) {
				for( auto const& StaticVariable : StructInfo->Static.Variables ) {
					Stream << "\t" << StaticVariable->Name << " : " << StaticVariable->Type->Definition.GetMangledName() << " static\n";
				}
				for( auto const& MemberVariable : StructInfo->Member.Variables ) {
					Stream << "\t" << MemberVariable->Name << " : " << MemberVariable->Type->Definition.GetMangledName() << "\n";
				}
				Stream << "\n";
			}
			Stream << "}\n";

		} else {
			Stream << Info->Definition.GetMangledName();
		}
	}

	void TypeInfo::PrintAll( std::ostream& Stream ) {
		Demangler D;
		for( Reflection::TypeInfo const* Info : GlobalTypeCollection ) {
			if( Info ) {
				Stream << std::hex << std::right << std::setw( 8 ) << std::setfill( '0' ) << Info->UniqueID
					<< std::dec << std::setw( 0 );
				Stream << " " << D.Demangle( *Info ) << " (" << Info->Definition.Size << ")" << std::endl;
			} else {
				Stream << "{{INVALID TYPE}}" << std::endl;
			}
		}
	}

	std::deque<TypeInfo const*>::const_iterator TypeInfo::GetTypeInfoIterator() {
		return GlobalTypeCollection.begin();
	}

	TypeInfo const* TypeInfo::FindTypeByID( sid_t UniqueID ) {
		for( TypeInfo const* Info : GlobalTypeCollection ) {
			if( Info && Info->UniqueID == UniqueID ) {
				return Info;
			}
		}
		return nullptr;
	}

	TypeInfo::TypeInfo(
		ETypeClassification InClassification, sid_t InUniqueID, CompilerDefinition InDefinition,
		char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer
	)
	: Classification( InClassification ), UniqueID( InUniqueID ), Definition( InDefinition )
	, Description( InDescription ), Flags( InFlags ), Serializer( InSerializer )
	{
		GlobalTypeCollection.push_back( this );
	}
}
