#include "Engine/Reflection/TypeUtility.h"

namespace Reflection {
	#define CASE_ENUM(Value, Display) case ETypeClassification::Value: return #Display;
	std::string_view GetClassificationIdentifier(ETypeClassification classification) {
		switch (classification) {
			default:
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
			CASE_ENUM(Reference, REFR);
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

		std::ostream_iterator<char> out{ stream };
		
		std::format_to(out, "{} [{}] {}"sv, type->id, GetClassificationIdentifier(type->classification), type->GetName());

		//Print the memory parameters
		if (flags.Has(EDebugPrintFlags::IncludeMetrics)) {
			std::format_to(out, " ({}:{})"sv, type->memory.size, type->memory.alignment);
		}

		//Print detailed info for this type, depending on the kind
		if (flags.Has(EDebugPrintFlags::DetailedInfo)) {
			if (StructTypeInfo const* structType = type->As<StructTypeInfo>()) {
				//Print variables
				if (structType->variables.size() > 0) {
					std::format_to(out, " variables:\n");
					for (auto const& variable : structType->variables) {
						std::format_to(out, "\t{} : {}\n"sv, variable.name, variable.type->GetName());
					}
				}
			}
		}

		//Finish with a newline
		stream << std::endl;
	}
}
