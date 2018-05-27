#include <string>
#include <string_view>
#include "Reflection/TypeUtility.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/ConstantInfo.h"
#include "Reflection/Components/VariableInfo.h"

namespace Reflection {
	void PrintType( std::ostream& Stream, TypeInfo const& Type ) {
		if( Type.Serializer ) {
			Stream << "[Serializable]\n";
		}
		if( StructTypeInfo const* StructInfo = Type.As<StructTypeInfo>() ) {
			Stream << "struct " << StructInfo->Name << " {\n";
			for( auto const& StaticConstant : StructInfo->StaticConstants ) {
				Stream << "\t" << StaticConstant->Name << " : " << StaticConstant->Type->Name << " const static\n";
			}
			for( auto const& MemberConstant : StructInfo->MemberConstants ) {
				Stream << "\t" << MemberConstant->Name << " : " << MemberConstant->Type->Name << " const\n";
			}
			Stream << "\n";
			for( auto const& StaticVariable : StructInfo->StaticVariables ) {
				Stream << "\t" << StaticVariable->Name << " : " << StaticVariable->Type->Name << " static\n";
			}
			for( auto const& MemberVariable : StructInfo->MemberVariables ) {
				Stream << "\t" << MemberVariable->Name << " : " << MemberVariable->Type->Name << "\n";
			}
			Stream << "}\n";

		} else {
			Stream << Type.Name;
		}
	}
}
