#pragma once
#include <initializer_list>
#include <string>
#include <string_view>
#include <ostream>

namespace Reflection {
	struct TypeInfo;

	/** Make a name string for a template that is using the provided types, in order */
	std::string MakeTemplateName( std::string_view TemplateName, std::initializer_list<TypeInfo const*> const& Types );
	/** Make a name string for a fixed-size standard array */
	std::string MakeArrayName( TypeInfo const* Type, size_t Size );

	/** Print the type information to the output stream */
	void PrintType( std::ostream& Stream, TypeInfo const& Type );
}
