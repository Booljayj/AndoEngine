#include "Engine/Reflection/TypeUtility.h"

namespace Reflection {
	#define CASE_ENUM(Value, Display) case ETypeClassification::Value: return #Display;
	std::string_view GetClassificationIdentifier(ETypeClassification classification) {
		switch (classification) {
			CASE_ENUM(Unknown, XXXX);
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
			stream << "{{INVALID TYPE}}"sv << std::endl;
			return;
		}

		//Print the unique ID of this type in hexadecimal
		std::ios_base::fmtflags f{ stream.flags() };
		stream << std::hex << std::right << std::setw(sizeof(type->id.low)) << std::setfill('0') << type->id.low << '-' << type->id.high;
		stream.flags(f);

		//Print the kind of TypeInfo this is.
		stream << " ["sv << GetClassificationIdentifier(type->classification) << "] "sv;

		//Print the name of the type
		stream << type->GetName();

		//Print the memory parameters
		if (flags.Has(EDebugPrintFlags::IncludeMetrics)) {
			stream << " ("sv << type->memory.size << ':' << type->memory.alignment << ")"sv;
		}

		//Print detailed info for this type, depending on the kind
		if (flags.Has(EDebugPrintFlags::DetailedInfo)) {
			if (StructTypeInfo const* structType = type->As<StructTypeInfo>()) {
				//Print variables
				if (structType->variables.size() > 0) {
					for (auto const& variable : structType->variables) {
						stream << '\t' << variable.name << " : "sv << variable.type->GetName() << "\n"sv;
					}
				}
			}
		}

		//Finish with a newline
		stream << std::endl;
	}
}
