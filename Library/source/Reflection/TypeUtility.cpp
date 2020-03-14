#include <iomanip>
#include "Reflection/TypeUtility.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	//A static demangler used by the debug functions in TypeUtility. This mostly exists for convenience.
	Demangler debugDemangler{};

	#define CASE_ENUM(Value, Display) case ETypeClassification::Value: return #Display;
	std::string_view GetClassificationIdentifier(ETypeClassification classification) {
		switch (classification) {
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

	void DebugPrint(TypeInfo const* type, std::ostream& stream, FDebugPrintFlags flags) {
		if (!type) {
			stream << "{{INVALID TYPE}}" << std::endl;
			return;
		}

		//Print the unique ID of this type in hexadecimal
		std::ios_base::fmtflags f{ stream.flags() };
		stream << std::hex << std::right << std::setw(sizeof(type->id.low)) << std::setfill('0') << type->id.low << "-" << type->id.high;
		stream.flags(f);

		//Print the kind of TypeInfo this is.
		stream << " [" << GetClassificationIdentifier(type->classification) << "]";

		//Print additional metrics, like size and whether it can be serialized
		if (flags.Has(EDebugPrintFlags::IncludeMetrics)) {
			stream << " (" << type->def.size << ":" << type->def.alignment;
			if (type->serializer) {
				stream << ", Serializable)";
			} else {
				stream << ")";
			}
		}

		//Print the name of the type, optionally demangled
		if (flags.Has(EDebugPrintFlags::DemangleName)) {
			stream << " " << debugDemangler.Demangle(*type);
		} else {
			stream << " " << type->def.GetMangledName();
		}

		//Print detailed info for this type, depending on the kind
		if (flags.Has(EDebugPrintFlags::DetailedInfo)) {
			if (StructTypeInfo const* structType = type->As<StructTypeInfo>()) {
				//Print constants
				if (structType->statics.constants.size() > 0 || structType->members.constants.size() > 0) {
					for (auto const& staticConstant : structType->statics.constants) {
						stream << "\t" << staticConstant.name << " : " << staticConstant.type->def.GetMangledName() << " const static\n";
					}
					for (auto const& memberConstant : structType->members.constants) {
						stream << "\t" << memberConstant.name << " : " << memberConstant.type->def.GetMangledName() << " const\n";
					}
					stream << std::endl;
				}
				//Print variables
				if (structType->statics.variables.size() > 0 || structType->members.variables.size() > 0) {
					for (auto const& staticVariable : structType->statics.variables) {
						stream << "\t" << staticVariable.name << " : " << staticVariable.type->def.GetMangledName() << " static\n";
					}
					for (auto const& memberVariable : structType->members.variables) {
						stream << "\t" << memberVariable.name << " : " << memberVariable.type->def.GetMangledName() << "\n";
					}
					stream << std::endl;
				}
			}
		}

		//Finish with a newline
		stream << std::endl;
	}
}
