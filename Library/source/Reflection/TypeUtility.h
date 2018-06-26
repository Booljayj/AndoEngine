#pragma once
#include <initializer_list>
#include <string>
#include <string_view>
#include <ostream>
#include <sstream>
#include "Reflection/BaseResolver.h"

namespace Reflection {
	struct TypeInfo;

	/** Make a name string for a template that is using the provided types, in order */
	template< typename... TYPES >
	std::string MakeTemplateName( std::string_view TemplateName ) {
		std::stringstream Stream;
		std::array<std::string_view, sizeof...( TYPES )> ArgumentNames = { { TypeResolver<TYPES>::GetName() ... } };
		//Build the name from each part in the same way it would be written in a code file
		Stream << TemplateName << "<";
		for( std::string_view const& ArgumentName : ArgumentNames ) {
			Stream << ArgumentName << ",";
		}
		//Convert the last character from ',' to '>'
		Stream.seekp( -1, std::ios_base::end ) << ">";
		return Stream.str();
	}
}
