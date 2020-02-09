#include <iomanip>
#include "Reflection/TypeUtility.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	//A static demangler used by the debug functions in TypeUtility. This mostly exists for convenience.
	Demangler DebugDemangler{};

	#define CASE_ENUM( __VALUE__, __DISPLAY__ ) case ETypeClassification::__VALUE__: return #__DISPLAY__;
	std::string_view GetClassificationIdentifier(ETypeClassification Classification) {
		switch (Classification) {
			CASE_ENUM(Primitive, PRIM);
			CASE_ENUM(Struct, STRU);
			CASE_ENUM(Alias, ALIA);
			CASE_ENUM(Enumeration, ENUM);
			CASE_ENUM(Flags, FLAG);
			CASE_ENUM(Array, ARRY);
			CASE_ENUM(Map, MAP_);
			CASE_ENUM(Set, SET_);
			CASE_ENUM(Poly, POLY);
			CASE_ENUM(Tuple, TUPL);
			CASE_ENUM(Variant, VARI);
		}
	}
	#undef CASE_ENUM

	void DebugPrint(TypeInfo const* Info, std::ostream& Stream, FDebugPrintFlags Flags) {
		if (!Info) {
			Stream << "{{INVALID TYPE}}" << std::endl;
			return;
		}

		//Print the unique ID of this type in hexadecimal
		std::ios_base::fmtflags f{ Stream.flags() };
		Stream << std::hex << std::right << std::setw(sizeof(Info->UniqueID.low)) << std::setfill('0') << Info->UniqueID.low << "-" << Info->UniqueID.high;
		Stream.flags(f);

		//Print the kind of TypeInfo this is.
		Stream << " [" << GetClassificationIdentifier(Info->Classification) << "]";

		//Print additional metrics, like size and whether it can be serialized
		if (Flags * FDebugPrintFlags::IncludeMetrics) {
			Stream << " (" << Info->Definition.Size << ":" << Info->Definition.Alignment;
			if (Info->Serializer) {
				Stream << ", Serializable)";
			} else {
				Stream << ")";
			}
		}

		//Print the name of the type, optionally demangled
		if (Flags * FDebugPrintFlags::DemangleName) {
			Stream << " " << DebugDemangler.Demangle(*Info);
		} else {
			Stream << " " << Info->Definition.GetMangledName();
		}

		//Print detailed info for this type, depending on the kind
		if (Flags * FDebugPrintFlags::DetailedInfo) {
			if (StructTypeInfo const* StructInfo = Info->As<StructTypeInfo>()) {
				//Print constants
				if (StructInfo->Static.Constants.Size() > 0 || StructInfo->Member.Constants.Size() > 0) {
					for (auto const& StaticConstant : StructInfo->Static.Constants) {
						Stream << "\t" << StaticConstant->Name << " : " << StaticConstant->Type->Definition.GetMangledName() << " const static\n";
					}
					for (auto const& MemberConstant : StructInfo->Member.Constants) {
						Stream << "\t" << MemberConstant->Name << " : " << MemberConstant->Type->Definition.GetMangledName() << " const\n";
					}
					Stream << std::endl;
				}
				//Print variables
				if (StructInfo->Static.Variables.Size() > 0 || StructInfo->Member.Variables.Size() > 0) {
					for (auto const& StaticVariable : StructInfo->Static.Variables) {
						Stream << "\t" << StaticVariable->Name << " : " << StaticVariable->Type->Definition.GetMangledName() << " static\n";
					}
					for (auto const& MemberVariable : StructInfo->Member.Variables) {
						Stream << "\t" << MemberVariable->Name << " : " << MemberVariable->Type->Definition.GetMangledName() << "\n";
					}
					Stream << std::endl;
				}
			}
		}

		//Finish with a newline
		Stream << std::endl;
	}
}
