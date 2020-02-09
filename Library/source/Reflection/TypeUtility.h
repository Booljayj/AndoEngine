#pragma once
#include <initializer_list>
#include <string>
#include <string_view>
#include <ostream>
#include <sstream>
#include <cxxabi.h>
#include "Engine/Flags.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	/** Converts compiler-generated names, which are stored in the TypeInfo for types, into human-readable ones similar to how they were originally declared in source */
	struct Demangler {
		Demangler() = default;
		~Demangler() {
			if (buffer != nullptr) std::free(buffer);
		}

		/** Demangle the name of a type and return a view of hte results */
		std::string Demangle(TypeInfo const& type) {
			//Perform the demangling. Will return nullptr if the demangling fails, otherwise will return the buffer that contains the name,
			// which may be the same as the input buffer if there was enough space. Or a brand new one if not
			int result = 1;
			char* newBuffer = abi::__cxa_demangle(type.def.GetMangledName(), buffer, &length, &result);
			if (newBuffer != nullptr) buffer = newBuffer;

			if (result == 0) return std::string{buffer};
			else return std::string{};
		}

	private:
		char* buffer = nullptr;
		size_t length = 0;
	};

	/** Returns an identifier code suitable to display the classification */
	std::string_view GetClassificationIdentifier(ETypeClassification classification);

	enum class EDebugPrintFlags : uint8_t {
		DemangleName,
		IncludeMetrics,
		DetailedInfo,
	};
	using FDebugPrintFlags = TFlags<EDebugPrintFlags>;

	/** Print a description of a TypeInfo to a stream for debugging purposes. */
	void DebugPrint(TypeInfo const* type, std::ostream& stream, FDebugPrintFlags flags = FDebugPrintFlags::None);
}
