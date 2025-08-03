#pragma once
#include "Engine/Core.h"
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/Threads.h"

struct ILogDevice;
struct LogCategory;
struct LogMessageQueue;

/** Delegates log output to various devices, which handle displaying and/or storing that output */
struct Logger {
	/** Get the static logger instance */
	static Logger& Get() { return instance; }

	Logger();
	~Logger();

	/** Push a message through this logger. Mainly used through logging macros. */
	template<typename ArgType, typename... OtherArgTypes>
	inline void Push(LogCategory const& category, ELogVerbosity verbosity, std::source_location location, std::format_string<ArgType, OtherArgTypes...> format, ArgType argument, OtherArgTypes... arguments) noexcept {
		PushFormatted(category, verbosity, location, format.get(), std::make_format_args(argument, arguments...));
	}

	/** Push a message through this logger. Mainly used through logging macros. */
	inline void Push(LogCategory const& category, ELogVerbosity verbosity, std::source_location location, std::format_string<> format) noexcept {
		PushUnformatted(category, verbosity, location, format.get());
	}

	/** Add new devices that will handle output */
	void AddDevices(std::span<std::shared_ptr<ILogDevice> const> view);
	inline void AddDevices(std::shared_ptr<ILogDevice> device) { AddDevices(std::span{ &device, 1 }); }

	/** Remove previously created devices from this logger */
	void RemoveDevices(std::span<std::shared_ptr<ILogDevice> const> view);
	inline void RemoveDevices(std::shared_ptr<ILogDevice> device) { RemoveDevices(std::span{ &device, 1 }); }

private:
	static Logger instance;
	static thread_local std::string scratch;
	
	struct {
		std::mutex queue;
		std::mutex thread;
	} mutex;

	std::unique_ptr<LogMessageQueue> queue;
	std::condition_variable cv;

	std::vector<std::shared_ptr<ILogDevice>> devices;
	std::optional<std::jthread> thread;

	void PushFormatted(LogCategory const& category, ELogVerbosity verbosity, std::source_location location, std::string_view format, std::format_args const& args) noexcept;
	void PushUnformatted(LogCategory const& category, ELogVerbosity verbosity, std::source_location location, std::string_view message) noexcept;
	
	void StopWorkerThread();
	void RestartWorkerThread();
};
