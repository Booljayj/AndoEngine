#pragma once
#include "Engine/TemporaryStrings.h"

/** Format a string within the temporary buffer and return a view to it. The returned string is only valid until the temporary buffer is reset. */
template<typename... ArgTypes>
constexpr std::string_view Format(std::format_string<ArgTypes...> format, ArgTypes&& ... args) {
	using namespace std;

	if constexpr (sizeof...(ArgTypes) > 0) {
		Buffer& buffer = ThreadBuffer::Get();
		size_t const available = buffer.GetAvailable();

		char* const begin = buffer.GetCursor();
		auto const result = format_to_n(back_inserter(buffer), available, format, forward<ArgTypes>(args)...);

		return string_view{ begin, static_cast<uint32_t>(result.size) };

	} else {
		return format.get();
	}
}

/** A type that can be constructed with only a string parameter */
template<typename T>
concept ConstructibleFromString =
	std::constructible_from<std::string_view> or
	std::constructible_from<std::string> or
	std::constructible_from<char const*>;

/** Given a type that can be constructed with a string, construct it with a formatted string created with the temporary allocator */
template<ConstructibleFromString ReturnType, typename... ArgTypes>
constexpr ReturnType FormatType(std::format_string<ArgTypes...> format, ArgTypes&&... arguments) {
	using namespace std;

	if constexpr (sizeof...(ArgTypes) > 0) {
		//One or more formatting parameters are present, so we need to perform formatting
		const auto view = Format(format, forward<ArgTypes>(arguments)...);

		if constexpr (constructible_from<ReturnType, string_view>) return ReturnType{ view };
		else if constexpr (constructible_from<ReturnType, string>) return ReturnType{ string{ view } };
		else return ReturnType{ view.data() };

	} else {
		//No formatting parameters are present, so we can skip formatting and just use the provided format string
		if constexpr (constructible_from<ReturnType, string_view>) return ReturnType{ format.get() };
		else if constexpr (constructible_from<ReturnType, string>) return ReturnType{ string{ format.get() } };
		else return ReturnType{ format.get().data() };
	}
}
