#pragma once
#include "Engine/Logging/LogMessage.h"
#include "Engine/Core.h"
#include "Engine/SmartPointers.h"
#include "Engine/String.h"
#include "Engine/Vector.h"

/** A device created within a Logger that is responsible for processing output messages. */
struct ILogDevice {
	virtual ~ILogDevice() = default;
	
	/** Process a log message */
	virtual void ProcessMessage(LogMessageHeader const& header, std::string const& message) noexcept = 0;
};

/** Allow the collection of devices to process the queue of log messages, emptying the queue when finished */
std::vector<std::shared_ptr<ILogDevice>>& operator<<(std::vector<std::shared_ptr<ILogDevice>>& devices, LogMessageQueue& queue);

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
