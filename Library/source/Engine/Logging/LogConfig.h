#pragma once
#include "Engine/Logging/LogVerbosity.h"

/** Contains methods used to configure log behavior, usually controlled by compiler-assigned flags */
namespace LogConfig {
	/** True if log messages should include the source location of the message */
	constexpr inline bool IncludeLocations() {
#if defined LOG_INCLUDE_LOCATIONS
		return true;
#else
		return false;
#endif
	}

	/** True if the verbosity level should be compiled */
	constexpr inline bool IsCompiled(ELogVerbosity verbosity) {
#if defined LOG_DISABLE
		return false;
#elif defined MAXIMUM_COMPILED_LOG_VERBOSITY
		return verbosity <= ELogVerbosity::MAXIMUM_COMPILED_LOG_VERBOSITY;
#else
		return true;
#endif
	}
}
