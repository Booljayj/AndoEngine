#pragma once
#include "Engine/Logging/LogCategory.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"
#include "Engine/Utility.h"

#ifndef MINIMUM_LOG_VERBOSITY
#define MINIMUM_LOG_VERBOSITY ELogVerbosity::Debug
#endif

#ifndef LOG_DISABLE
namespace LoggingInternal {
	/** Helper template to resolve compiler-removed output */
	template<ELogVerbosity Verbosity>
	inline typename std::enable_if<(Verbosity >= MINIMUM_LOG_VERBOSITY)>::type
	LogHelper(LogCategory const& category, char const* location, char const* message) {
		Logger::Get().Output(category, Verbosity, location, message);
	}
	template<ELogVerbosity Verbosity>
	inline typename std::enable_if<(Verbosity < MINIMUM_LOG_VERBOSITY)>::type
	LogHelper(LogCategory const& category, char const* location, char const* message) { /** no-op, removed by compiler */ }

	/** Helper template to resolve compiler-removed formatted output */
	template<ELogVerbosity Verbosity, typename... ArgTypes>
	inline typename std::enable_if<(Verbosity >= MINIMUM_LOG_VERBOSITY)>::type
	LogFormattedHelper(LogCategory const& category, char const* location, char const* message, ArgTypes&&... args) {
		Logger::Get().Output(category, Verbosity, location, l_printf(*threadHeapBuffer, message, std::forward<ArgTypes>(args)...));
	}
	template<ELogVerbosity Verbosity, typename... ArgTypes>
	inline typename std::enable_if<(Verbosity < MINIMUM_LOG_VERBOSITY)>::type
	LogFormattedHelper(LogCategory const& category, char const* location, char const* message, ArgTypes&&... args) { /** no-op, removed by compiler */ }
}

/** Expands to a string that describes the file location where it appears */
#define LOCATION __FILE__ ":" STRINGIFY_MACRO(__LINE__)

/** Log a message to the current context's logger */
#define LOG(Category, Verbosity, Message) LoggingInternal::LogHelper<ELogVerbosity::Verbosity>(Log ## Category, LOCATION, Message)
/** Log a formatted message to the current context's logger */
#define LOGF(Category, Verbosity, Message, ...) LoggingInternal::LogFormattedHelper<ELogVerbosity::Verbosity>(Log ## Category, LOCATION, Message, __VA_ARGS__)

#else
#define LOG(Category, Verbosity, Message)
#define LOGF(Category, Verbosity, Message, ...)

#endif
