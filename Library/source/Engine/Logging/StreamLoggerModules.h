#pragma once
#include "Engine/Logging/Logger.h"
#include "Engine/StandardTypes.h"

/** Writes output to a stream */
struct StreamLoggerModule : public LoggerModule {
public:
	StreamLoggerModule(std::ostream& stream);
	virtual void ProcessMessage(LogOutputData const& outputData) override;

protected:
	std::ostream* streamPtr;
};
