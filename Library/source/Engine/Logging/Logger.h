#pragma once
#include "Engine/Logging/LogOutput.h"
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/StandardTypes.h"

struct LogCategory;
struct LogOutputDevice;

/** Delegates log output to various devices, which handle displaying and/or storing that output */
struct Logger {
	/** Get the static logger instance */
	static Logger& Get() { return instance; }

	/** Output a message through this logger. Mainly used through logging macros. */
	void Output(LogCategory const& category, ELogVerbosity verbosity, SourceLocation location, std::string_view message);
	void Output(LogCategory const& category, ELogVerbosity verbosity, std::string_view message);

	/** Add a new device which will handle output */
	template<typename ModuleType, typename... ArgTypes>
	size_t CreateDevice(ArgTypes... args) {
		std::scoped_lock const lock{ mutex };

		size_t newID = currentID++;
		devices.push_back(make_tuple(newID, std::make_unique<ModuleType>(std::forward(args)...)));
		return newID;
	}

	/** Destroy a previously created device from this logger */
	void DestroyDevice(size_t id);

private:
	Logger() = default;

	static Logger instance;

	std::mutex mutex;
	std::vector<std::tuple<size_t, std::unique_ptr<LogOutputDevice>>> devices;
	size_t currentID = 0;
};
