#pragma once
#include "Engine/Logging/LogMessage.h"
#include "Engine/StandardTypes.h"

/** A device created within a Logger that is responsible for processing output messages. */
struct ILogDevice {
	virtual ~ILogDevice() = default;
	
	/** Process a log message */
	virtual void ProcessMessage(LogMessageHeader const& header, std::string const& message) noexcept = 0;
};

/** A managed collection of log devices */
using ILogDeviceCollection = std::vector<std::shared_ptr<ILogDevice>>;
/** A view to a managed collection of log devices */
using ILogDeviceView = std::span<std::shared_ptr<ILogDevice> const>;

/** Allow the collection of devices to process the queue of log messages, emptying the queue when finished */
ILogDeviceCollection& operator<<(ILogDeviceCollection& devices, LogMessageQueue& queue);

/** Writes output to the standard output streams with terminal formatting */
struct TerminalLogDevice : public ILogDevice {
	TerminalLogDevice();
	virtual void ProcessMessage(LogMessageHeader const& header, std::string const& message) noexcept override;

private:
	std::string buffer;
};

/** Writes output to an arbitrary stream */
struct StreamLogDevice : public ILogDevice {
	StreamLogDevice(std::ostream& inStream);
	virtual void ProcessMessage(LogMessageHeader const& header, std::string const& message) noexcept override;

protected:
	std::ostream& stream;
};

/** Writes output to a file */
struct FileLogDevice : public ILogDevice {
	FileLogDevice(std::string_view fileName);
	virtual void ProcessMessage(LogMessageHeader const& header, std::string const& message) noexcept override;

protected:
	std::fstream stream;
};
