#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/Temporary.h"

template<typename ExceptionType, typename ArgType, typename... OtherArgTypes>
	requires std::is_base_of_v<std::exception, ExceptionType>
constexpr ExceptionType MakeException(std::format_string<ArgType, OtherArgTypes...> format, ArgType&& argument, OtherArgTypes&&... arguments) {
	Buffer& buffer = ThreadBuffer::Get();
	size_t const available = buffer.GetAvailable();

	char* const begin = buffer.GetCursor();
	(void)std::format_to_n(std::back_inserter(buffer), available, format, std::forward<ArgType>(argument), std::forward<OtherArgTypes>(arguments)...);

	return ExceptionType{ begin };
}

template<typename ExceptionType>
	requires std::is_base_of_v<std::exception, ExceptionType>
constexpr ExceptionType MakeException(std::format_string<> format) {
	return ExceptionType{ format.get().data() };
}
