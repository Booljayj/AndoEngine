#pragma once
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/Logging/LoggerModule.h"
#include "Engine/STL.h"

struct LogCategory;
struct LogOutputData;

/** An object which handles program output from within a single thread */
struct Logger {
	std::vector<std::shared_ptr<LoggerModule>> modules;

	/** Output a message through this logger. Mainly used through logging macros */
	void Output(LogCategory const& category, ELogVerbosity verbosity, std::string_view location, std::string_view message) const;

	/** Add the provided module to this logger to process output */
	void AddModule(std::shared_ptr<LoggerModule> const& module);

	template<typename ModuleType, typename... ArgTypes>
	void AddModule(ArgTypes... args) {
		AddModule(std::make_shared<ModuleType>(std::forward(args)...));
	}

	/** Remove a previously added module from this logger */
	void RemoveModule(std::shared_ptr<LoggerModule> const& module);
};
