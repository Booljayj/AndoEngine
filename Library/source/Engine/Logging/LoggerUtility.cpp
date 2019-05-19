#include "Engine/Logging/LoggerUtility.h"
#include "Engine/TerminalColors.h"

namespace LoggerUtility {
	char const* GetText( ELogVerbosity const Verbosity ) noexcept {
		switch(Verbosity) {
			case ELogVerbosity::Debug: return "Debug: ";
			case ELogVerbosity::Info: return "Info: ";
			case ELogVerbosity::Warning: return "Warning: ";
			case ELogVerbosity::Error: return "Error: ";
			default: return "!INVALID VERBOSITY!";
		}
	}

	char const* GetTerminalColor( ELogVerbosity const Verbosity ) noexcept {
		switch(Verbosity) {
			case ELogVerbosity::Debug: return TERM_Cyan;
			case ELogVerbosity::Info: return TERM_NoColor;
			case ELogVerbosity::Warning: return TERM_Yellow;
			case ELogVerbosity::Error: return TERM_Red;
			default: return TERM_Purple;
		}
	}
}