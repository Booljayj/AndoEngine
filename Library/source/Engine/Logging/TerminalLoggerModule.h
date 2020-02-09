#include <mutex>
#include "Engine/Logging/LoggerModule.h"

/** A logger module which prints to standard output streams with terminal formatting */
struct TerminalLoggerModule : public LoggerModule {
protected:
	virtual void InternalProcessMessage(LogOutputData const& outputData) override;
};
