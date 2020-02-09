#pragma once
#include <string_view>
#include <type_traits>
#include "Engine/Context.h"
#include "Engine/Logging/LogCategory.h"
#include "Engine/LinearStrings.h"

#ifndef MINIMUM_LOG_VERBOSITY
#define MINIMUM_LOG_VERBOSITY ELogVerbosity::Debug
#endif

#ifndef LOG_DISABLE
namespace LoggingInternal {
	/** Helper template to resolve compiler-removed output */
	template<ELogVerbosity Verbosity>
	inline typename std::enable_if<(Verbosity >= MINIMUM_LOG_VERBOSITY)>::type
	LogHelper(CTX_ARG, LogCategory const& category, char const* location, char const* message) {
		CTX.log.Output(category, Verbosity, location, message);
	}
	template<ELogVerbosity Verbosity>
	inline typename std::enable_if<(Verbosity < MINIMUM_LOG_VERBOSITY)>::type
	LogHelper(CTX_ARG, LogCategory const& category, char const* location, char const* message) { /** no-op, removed by compiler */ }

	/** Helper template to resolve compiler-removed formatted output */
	template<ELogVerbosity Verbosity, typename... ArgTypes>
	inline typename std::enable_if<(Verbosity >= MINIMUM_LOG_VERBOSITY)>::type
	LogFormattedHelper(CTX_ARG, LogCategory const& category, char const* location, char const* message, ArgTypes&&... args) {
		CTX.log.Output(category, Verbosity, location, l_printf(CTX.temp, message, std::forward<ArgTypes>(args)...));
	}
	template<ELogVerbosity Verbosity, typename... ArgTypes>
	inline typename std::enable_if<(Verbosity < MINIMUM_LOG_VERBOSITY)>::type
	LogFormattedHelper(CTX_ARG, LogCategory const& category, char const* location, char const* message, ArgTypes&&... args) { /** no-op, removed by compiler */ }
}

#define S1(X) #X
#define S2(X) S1(X)
#define LOCATION (__FILE__ ":" S2(__LINE__))

/** Log a message to the current context's logger */
#define LOG(CAT, VERBOSITY, MESSAGE) LoggingInternal::LogHelper<ELogVerbosity::VERBOSITY>(CTX, CAT, LOCATION, MESSAGE)
/** Log a formatted message to the current context's logger */
#define LOGF(CAT, VERBOSITY, MESSAGE, ...) LoggingInternal::LogFormattedHelper<ELogVerbosity::VERBOSITY>(CTX, CAT, LOCATION, MESSAGE, __VA_ARGS__)

#else
#define LOG(CAT, VERBOSITY, MESSAGE)
#define LOGF(CAT, VERBOSITY, MESSAGE, ...)

#endif
