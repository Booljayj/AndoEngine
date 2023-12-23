#pragma once
#include "Engine/Logging/LogMessage.h"
#include "Engine/Logging/LogDevice.h"
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/StandardTypes.h"

struct LogCategory;

/** Delegates log output to various devices, which handle displaying and/or storing that output */
struct Logger {
	/** Get the static logger instance */
	static Logger& Get() { return instance; }

	Logger();
	~Logger();

	/** Push a message through this logger. Mainly used through logging macros. */
	template<typename... ArgTypes>
	inline void Push(LogCategory const& category, ELogVerbosity verbosity, std::source_location location, std::format_string<ArgTypes...> format, ArgTypes... arguments) noexcept {
		PushInternal(category, verbosity, location, format.get(), std::make_format_args(arguments...));
	}

	/** Add new devices that will handle output */
	void AddDevices(ILogDeviceView view);
	inline void AddDevices(std::shared_ptr<ILogDevice> device) { AddDevices(std::span{ &device, 1 }); }

	/** Remove previously created devices from this logger */
	void RemoveDevices(ILogDeviceView view);
	inline void RemoveDevices(std::shared_ptr<ILogDevice> device) { RemoveDevices(std::span{ &device, 1 }); }

private:
	static Logger instance;
	
	struct {
		std::mutex queue;
		std::mutex thread;
	} mutex;

	std::unique_ptr<LogMessageQueue> queue;
	std::condition_variable cv;

	ILogDeviceCollection devices;
	std::optional<std::jthread> thread;

	void PushInternal(LogCategory const& category, ELogVerbosity verbosity, std::source_location location, std::string_view format, std::format_args const& args) noexcept;
	
	void StopWorkerThread();
	void RestartWorkerThread();
};
