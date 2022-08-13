#pragma once
#include "Engine/Flags.h"
#include "Engine/Reflection.h"
#include "Engine/StandardTypes.h"

namespace Reflection {
	/** Returns an identifier code suitable to display the classification */
	std::string_view GetClassificationIdentifier(ETypeClassification classification);

	enum class EDebugPrintFlags : uint8_t {
		IncludeMetrics,
		DetailedInfo,
	};
	using FDebugPrintFlags = TFlags<EDebugPrintFlags>;

	/** Print a description of a TypeInfo to a stream for debugging purposes. */
	void DebugPrint(TypeInfo const* type, std::ostream& stream, FDebugPrintFlags flags = FDebugPrintFlags::None);
}
