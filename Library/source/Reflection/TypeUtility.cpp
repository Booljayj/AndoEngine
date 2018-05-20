#include <string>
#include <string_view>
#include "Reflection/TypeUtility.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	std::string MakeTemplateName( std::string_view TemplateName, std::initializer_list<TypeInfo const*> const& Types ) {
		std::ostringstream Stream;
		Stream << TemplateName << "<";
		for( TypeInfo const* Info : Types ) {
			Stream << Info->Name << ",";
		}
		Stream.seekp( -1, std::ios_base::cur ) << ">"; //back up one char to overwrite the last "," with a ">"
		return Stream.str();
	}

	std::string MakeArrayName( TypeInfo const* Type, size_t Size ) {
		std::ostringstream Stream;
		Stream << "std::array<" << Type->Name << "," << Size << ">";
		return Stream.str();
	}

	void PrintType( std::ostream& Stream, TypeInfo const& Type ) {
		if( Type.Serializer ) {
			Stream << "[Serializable]\n";
		}
		if( StructTypeInfo const* StructInfo = Type.As<StructTypeInfo>() ) {
			Stream << "struct " << StructInfo->GetName() << " {\n";
			for( auto const& StaticConstant : StructInfo->StaticConstants ) {
				Stream << "\t" << StaticConstant->Name << " : " << StaticConstant->Type->GetName() << " const static\n";
			}
			for( auto const& MemberConstant : StructInfo->MemberConstants ) {
				Stream << "\t" << MemberConstant->Name << " : " << MemberConstant->Type->GetName() << " const\n";
			}
			Stream << "\n";
			for( auto const& StaticVariable : StructInfo->StaticVariables ) {
				Stream << "\t" << StaticVariable->Name << " : " << StaticVariable->Type->GetName() << " static\n";
			}
			for( auto const& MemberVariable : StructInfo->MemberVariables ) {
				Stream << "\t" << MemberVariable->Name << " : " << MemberVariable->Type->GetName() << "\n";
			}
			Stream << "}\n";

		} else {
			Stream << Type.GetName();
		}
	}
}
