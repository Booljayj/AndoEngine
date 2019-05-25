#include "Engine/Logging/Logger.h"

namespace LoggerUtility {
	/** Gets an identifier to show in a message for a log verbosity */
	char const* GetText( ELogVerbosity const Verbosity ) noexcept;
	/** Gets the terminal color that can be used to colorize text for a log verbosity level */
	char const* GetTerminalColor( ELogVerbosity const Verbosity ) noexcept;
}
