#include "Engine/Logging/LogUtility.h"
#include "Engine/TerminalColors.h"

namespace LogUtility {
	std::string_view GetText(ELogVerbosity const verbosity) noexcept {
		switch (verbosity) {
			case ELogVerbosity::Debug: return std::string_view{"Debug: "};
			case ELogVerbosity::Info: return std::string_view{"Info: "};
			case ELogVerbosity::Warning: return std::string_view{"Warning: "};
			case ELogVerbosity::Error: return std::string_view{"Error: "};
			default: return std::string_view{"!INVALID VERBOSITY! "};
		}
	}

	std::string_view GetTerminalColor(ELogVerbosity const verbosity) noexcept {
		switch (verbosity) {
			case ELogVerbosity::Debug: return std::string_view{TERM_Cyan};
			case ELogVerbosity::Info: return std::string_view{TERM_NoColor};
			case ELogVerbosity::Warning: return std::string_view{TERM_Yellow};
			case ELogVerbosity::Error: return std::string_view{TERM_Red};
			default: return std::string_view{TERM_Purple};
		}
	}
}
