#pragma once
#include "Engine/Logging/Logger.h"

/** Writes output to the standard output streams with terminal formatting */
struct TerminalLoggerModule : public LoggerModule {
public:
	virtual void ProcessMessage(LogOutputData const& outputData) override;
};
