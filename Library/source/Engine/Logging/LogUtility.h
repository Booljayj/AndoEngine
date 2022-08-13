#pragma once
#include "Engine/Logging/Logger.h"
#include "Engine/StandardTypes.h"

namespace LogUtility {
	/** Gets an identifier to show in a message for a log verbosity */
	std::string_view GetText(ELogVerbosity const verbosity) noexcept;
	/** Gets the terminal color that can be used to colorize text for a log verbosity level */
	std::string_view GetTerminalColor(ELogVerbosity const verbosity) noexcept;
}
