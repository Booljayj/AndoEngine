#pragma once
#include "Engine/Logging/Logger.h"
#include "Engine/StandardTypes.h"

/** A thread-safe output device created within a Logger that is responsible for processing output messages. */
struct LogOutputDevice {
public:
	virtual ~LogOutputDevice() {}
	/** Process a message given to a logger. Thread safe. */
	virtual void ProcessOutput(LogOutput const& output) = 0;
};

/** Writes output to the standard output streams with terminal formatting */
struct TerminalOutputDevice : public LogOutputDevice {
public:
	TerminalOutputDevice();
	virtual void ProcessOutput(LogOutput const& output) override;
};

/** Writes output to an arbitrary stream */
struct StreamOutputDevice : public LogOutputDevice {
public:
	StreamOutputDevice(std::ostream& inStream);
	virtual void ProcessOutput(LogOutput const& output) override;

protected:
	std::ostream& stream;
};

/** Writes output to a file */
struct FileOutputDevice : public LogOutputDevice {
public:
	FileOutputDevice(std::string_view fileName);
	virtual void ProcessOutput(LogOutput const& output) override;

protected:
	std::fstream stream;
};
