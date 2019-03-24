#pragma once
#include <string_view>
#include <vector>
#include <memory>
#include "Engine/Logging/LogCategory.h"
#include "Engine/Logging/LogVerbosity.h"

/** Data for a log output operation */
struct LogOutputData {
	std::string_view Location;
	LogCategory const* Category;
	ELogVerbosity Verbosity;
	std::string_view Message;
};

/** A thread-safe module that is responsible for processing the messages sent to a logger object. Created as shared_ptrs that are used by relevant loggers. */
struct LoggerModule {
private:
	std::mutex AccessMutex;

protected:
	virtual void InternalProcessMessage( LogOutputData const& OutputData ) = 0;

public:
	virtual ~LoggerModule() {}

	/** Process a message given to a logger. Thread-safe. */
	void ProcessMessage( LogOutputData const& OutputData );
};

/** An object which handles program output from within a thread */
struct Logger {
	std::vector<std::shared_ptr<LoggerModule>> Modules;

	/** Output a message through this logger. Mainly used through logging macros */
	void Output( std::string_view Location, LogCategory const& Category, ELogVerbosity Verbosity, std::string_view Message ) const;
	void AddModule( std::shared_ptr<LoggerModule> const& Module );
	void RemoveModule( std::shared_ptr<LoggerModule> const& Module );
};
