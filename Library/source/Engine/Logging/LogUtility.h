#pragma once
#include "Engine/Logging/Logger.h"
#include "Engine/StandardTypes.h"

namespace LogUtility {
	/** Gets an identifier to show in a message for a log verbosity */
	std::string_view GetText(ELogVerbosity const verbosity) noexcept;
	/** Gets the terminal color that can be used to colorize text for a log verbosity level */
	std::string_view GetTerminalColor(ELogVerbosity const verbosity) noexcept;

	using SourceLineType = decltype(std::declval<std::source_location>().line());
	using SourceFileType = decltype(std::declval<std::source_location>().file_name());
	/** Returns a simplified source location useful for logging. Strips out unnecessary information. */
	consteval std::source_location GetSourceLocation(SourceLineType line = __builtin_LINE(), SourceFileType file = __builtin_FILE()) noexcept {
		std::string_view const filename = file;
		size_t const index = filename.find_last_of("\\/");
		if (index != std::string_view::npos && index < (filename.size() - 1)) {
			return std::source_location::current(line, 0, filename.substr(index + 1).data(), nullptr);
		} else {
			return std::source_location::current(line, 0, file, nullptr);
		}
	}
}
