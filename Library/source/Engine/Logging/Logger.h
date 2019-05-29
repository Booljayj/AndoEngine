#pragma once
#include <string_view>
#include <vector>
#include <memory>
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/Logging/LoggerModule.h"

struct LogCategory;
struct LogOutputData;

/** An object which handles program output from within a single thread */
struct Logger {
	std::vector<std::shared_ptr<LoggerModule>> Modules;

	/** Output a message through this logger. Mainly used through logging macros */
	void Output(std::string_view Location, LogCategory const& Category, ELogVerbosity Verbosity, std::string_view Message ) const;

	/** Add the provided module to this logger to process output */
	void AddModule( std::shared_ptr<LoggerModule> const& Module );

	template<typename TMODULE, typename... TARGS>
	void AddModule( TARGS... Args ) {
		AddModule( std::make_shared<TMODULE>(std::forward(Args)...) );
	}

	/** Remove a previously added module from this logger */
	void RemoveModule( std::shared_ptr<LoggerModule> const& Module );
};
