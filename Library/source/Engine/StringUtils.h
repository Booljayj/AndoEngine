#pragma once
#include "Engine/StandardTypes.h"

namespace StringUtils {
	constexpr int32_t ConvertStringToInteger(std::string_view string) {
		int32_t value = 0;
		std::from_chars_result const result = std::from_chars(string.data(), string.data() + string.size(), value);
		if (result.ec != std::errc{}) throw MakeException<std::runtime_error>("Could not convert string to number value");
		else return value;
	}

	/** Represents a string with an optional numeric suffix, which is extracted from a source string */
	struct DecomposedString {
		constexpr explicit DecomposedString(std::string_view source) {
			//Disregard the null terminator in all internal calculations. This makes searches and comparisons a little easier.
			if (source.size() > 0 && source.back() == '\0') source = source.substr(0, source.size() - 1);
			
			if (auto const index = FindNumberSuffixIndex(source)) {
				//The source string contains a number suffix
				body = source.substr(0, index.value());
				suffix = ConvertStringToInteger(source.substr(index.value()));

			} else {
				//The source string does not contain a number suffix
				body = source;
				suffix = 0;
			}
		}

		std::string_view body;
		uint32_t suffix = 0;

	private:
		constexpr std::optional<size_t> FindNumberSuffixIndex(std::string_view string) {
			auto const riter = std::find_if_not(string.crbegin(), string.crend(), [](char c) { return c >= '0' && c <= '9'; });
			if (riter != string.crbegin()) return std::distance(string.begin(), riter.base());
			else return std::nullopt;
		}
	};
}
