#pragma once
#include <initializer_list>
#include <string>
#include <string_view>
#include <ostream>
#include <sstream>
#include <cxxabi.h>
#include "Engine/StringID.h"
#include "Reflection/BaseResolver.h"

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
			char* NewBuffer = abi::__cxa_demangle( Info.Definition.MangledName, Buffer, &Length, &Result );
			if( NewBuffer != nullptr ) Buffer = NewBuffer;

			if( Result == 0 ) return std::string{ Buffer };
			else return std::string{};
		}

	private:
		char* Buffer = nullptr;
		size_t Length = 0;
		int Result = 1;
	};
}
