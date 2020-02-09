#pragma once
#include <mutex>

struct LogOutputData;

/** A thread-safe module that is responsible for processing the messages sent to a logger object. Created as shared_ptrs that are used by relevant loggers. */
struct LoggerModule {
public:
	virtual ~LoggerModule() {}
	/** Process a message given to a logger. Thread safe. */
	void ProcessMessage(LogOutputData const& outputData);

protected:
	/** Mutex that locks access to mutable data within this module. Used by normal interface functions, and can also be used by internal thread workers. */
	std::mutex accessMutex;
	virtual void InternalProcessMessage(LogOutputData const& outputData) = 0;
};
