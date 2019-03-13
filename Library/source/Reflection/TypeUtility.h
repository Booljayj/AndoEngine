#pragma once
#include <initializer_list>
#include <string>
#include <string_view>
#include <ostream>
#include <sstream>
#include <cxxabi.h>
#include "Engine/StringID.h"
#include "Engine/Flags.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	/** Converts compiler-generated names, which are stored in the TypeInfo for types, into human-readable ones similar to how they were originally declared in source */
	struct Demangler {
		Demangler() = default;
		~Demangler() {
			if( Buffer != nullptr ) std::free( Buffer );
		}

		/** Demangle the name of a type and return a view of hte results */
		std::string Demangle( TypeInfo const& Info ) {
			//Perform the demangling. Will return nullptr if the demangling fails, otherwise will return the buffer that contains the name,
			// which may be the same as the input buffer if there was enough space. Or a brand new one if
			int Result = 1;
			char* NewBuffer = abi::__cxa_demangle( Info.Definition.GetMangledName(), Buffer, &Length, &Result );
			if( NewBuffer != nullptr ) Buffer = NewBuffer;

			if( Result == 0 ) return std::string{ Buffer };
			else return std::string{};
		}

	private:
		char* Buffer = nullptr;
		size_t Length = 0;
	};

	/** Returns an identifier code suitable to display the classification */
	std::string_view GetClassificationIdentifier( ETypeClassification Classification );

	enum class FDebugPrintFlags : uint8_t {
		None = 0,
		DemangleName = 1 << 0,
		IncludeMetrics = 1 << 1,
		DetailedInfo = 1 << 2,
	};
	DEFINE_BITFLAG_OPERATORS( FDebugPrintFlags );

	/** Print a description of a TypeInfo to a stream for debugging purposes. */
	void DebugPrint( TypeInfo const* Info, std::ostream& Stream, FDebugPrintFlags Flags = FDebugPrintFlags::None );
}
