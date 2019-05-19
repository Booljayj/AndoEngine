#pragma once
#include <string_view>
#include <type_traits>
#include "Engine/Context.h"
#include "Engine/Logging/LogCategory.h"

#ifndef ENABLE_LOGGING
#define ENABLE_LOGGING 1
#endif

#ifndef MINIMUM_LOG_VERBOSITY
#define MINIMUM_LOG_VERBOSITY ELogVerbosity::Debug
#endif

#if ENABLE_LOGGING
namespace LoggingInternal {
	/** Helper template to resolve compiler-removed output */
	template<ELogVerbosity Verbosity>
	inline typename std::enable_if<Verbosity >= MINIMUM_LOG_VERBOSITY>::type
	LogHelper( CTX_ARG, char const* Location, LogCategory const& Category, char const* Message ) {
		CTX.Log.Output( Location, Category, Verbosity, Message );
	}
	template<ELogVerbosity Verbosity>
	inline typename std::enable_if<Verbosity < MINIMUM_LOG_VERBOSITY>::type
	LogHelper( CTX_ARG, char const* Location, LogCategory const& Category, char const* Message ) { /** no-op, removed by compiler */ }

	/** Helper template to resolve compiler-removed formatted output */
	template<ELogVerbosity Verbosity, typename... TARGS>
	inline typename std::enable_if<Verbosity >= MINIMUM_LOG_VERBOSITY>::type
	LogFormattedHelper( CTX_ARG, char const* Location, LogCategory const& Category, char const* Message, TARGS&&... Args ) {
		CTX.Log.Output( Location, Category, Verbosity, l_printf( CTX.Temp, Message, std::forward<TARGS>( Args )... ) );
	}
	template<ELogVerbosity Verbosity, typename... TARGS>
	inline typename std::enable_if<Verbosity < MINIMUM_LOG_VERBOSITY>::type
	LogFormattedHelper( CTX_ARG, char const* Location, LogCategory const& Category, char const* Message, TARGS&&... Args ) { /** no-op, removed by compiler */ }
}

#define S1( _X_ ) #_X_
#define S2( _X_ ) S1(_X_)
#define LOCATION (__FILE__ ":" S2(__LINE__))

/** Log a message to the current context's logger */
#define LOG( _CAT_, _VERBOSITY_, _MESSAGE_ ) LoggingInternal::LogHelper<ELogVerbosity::_VERBOSITY_>( CTX, LOCATION, _CAT_, _MESSAGE_ )
/** Log a formatted message to the current context's logger */
#define LOGF( _CAT_, _VERBOSITY_, _MESSAGE_, ... ) LoggingInternal::LogFormattedHelper<ELogVerbosity::_VERBOSITY_>( CTX, LOCATION, _CAT_, _MESSAGE_, __VA_ARGS__ )

#else
#define LOG( _CAT_, _VERBOSITY_, _MESSAGE_ )
#define LOGF( _CAT_, _VERBOSITY_, _MESSAGE_, ... )

#endif
