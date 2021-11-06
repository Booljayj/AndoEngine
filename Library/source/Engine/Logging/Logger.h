#pragma once
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/STL.h"

struct LogCategory;
struct LogOutputData;

/** A thread-safe module created within a Logger that is responsible for processing output messages. */
struct LoggerModule {
public:
	virtual ~LoggerModule() {}
	/** Process a message given to a logger. Thread safe. */
	virtual void ProcessMessage(LogOutputData const& outputData) = 0;
};

/** An object which delegates program output to various modules, which handle displaying and storing that output */
struct Logger {
	/** Get the static logger instance */
	static Logger& Get() { return instance; }

	/** Output a message through this logger. Mainly used through logging macros. */
	void Output(LogCategory const& category, ELogVerbosity verbosity, std::string_view location, std::string_view message);

	/** Add a new module which will handle output */
	template<typename ModuleType, typename... ArgTypes>
	size_t CreateModule(ArgTypes... args) {
		std::scoped_lock const lock{ mutex };

		size_t newID = currentID++;
		modules.push_back(make_tuple(newID, std::make_unique<ModuleType>(std::forward(args)...)));
		return newID;
	}

	/** Destroy a previously created module from this logger */
	void DestroyModule(size_t id);

private:
	Logger() = default;

	static Logger instance;

	std::mutex mutex;
	std::vector<std::tuple<size_t, std::unique_ptr<LoggerModule>>> modules;
	size_t currentID = 0;
};
