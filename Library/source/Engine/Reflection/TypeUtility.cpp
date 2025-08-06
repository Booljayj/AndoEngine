#include "Engine/Reflection/TypeUtility.h"
#include "Engine/String.h"

namespace Reflection {
	std::string_view GetClassificationIdentifier(ETypeClassification classification) {
		#define CASE_ENUM(Value, Display) case ETypeClassification::Value: return #Display;
		switch (classification) {
			default:
			CASE_ENUM(Unknown, XXXX);
			CASE_ENUM(Valueless, VLES);
			CASE_ENUM(Numeric, NUME);
			CASE_ENUM(Struct, STRU);
			CASE_ENUM(String, STNG);
			CASE_ENUM(Enum, ENUM);
			CASE_ENUM(Flags, FLAG);
			CASE_ENUM(Array, ARRY);
			CASE_ENUM(Map, MAP_);
			CASE_ENUM(Set, SET_);
			CASE_ENUM(Poly, POLY);
			CASE_ENUM(Reference, REFR);
			CASE_ENUM(Tuple, TUPL);
			CASE_ENUM(Variant, VARI);
		}
		#undef CASE_ENUM
	}

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
			if (StructTypeInfo const* structType = Cast<StructTypeInfo>(type)) {
				//Print variables
				if (structType->GetVariables().size() > 0) {
					std::format_to(out, " variables:\n");
					for (auto const& variable : structType->GetVariables()) {
						std::format_to(out, "\t{} : {}\n"sv, variable->name, variable->type->GetName());
					}
				}
			}
		}

		//Finish with a newline
		stream << std::endl;
	}
}
