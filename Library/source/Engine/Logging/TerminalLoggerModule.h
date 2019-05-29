#include <mutex>
#include "Engine/Logging/LoggerModule.h"

/** A logger module which prints to a stream with terminal formatting */
struct TerminalLoggerModule : public LoggerModule {
protected:
	virtual void InternalProcessMessage( LogOutputData const& OutputData ) override;
};
