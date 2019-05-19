#pragma once
#include <string_view>
#include <vector>
#include <memory>
#include "Engine/Logging/LogCategory.h"
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/LinearAllocator.h"
#include "Engine/TerminalColors.h"
#include "Engine/TimeStamp.h"

/** Data for a log output operation */
struct LogOutputData {
	TimeStamp TimeStamp;
	LogCategory const* Category;
	ELogVerbosity Verbosity;
	std::string_view Location;
	std::string_view Message;
};
std::ostream& operator<<( std::ostream& Stream, LogOutputData const& OutputData );

/** A thread-safe module that is responsible for processing the messages sent to a logger object. Created as shared_ptrs that are used by relevant loggers. */
struct LoggerModule {
public:
	virtual ~LoggerModule() {}
	/** Process a message given to a logger. Thread safe. */
	void ProcessMessage( LogOutputData const& OutputData );

protected:
	/** Mutex that locks access to mutable data within this module. Used by normal interface functions, and can also be used by internal thread workers. */
	std::mutex AccessMutex;
	virtual void InternalProcessMessage( LogOutputData const& OutputData ) = 0;
};

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
