#pragma once
#include "Engine/Logging/LoggerModule.h"
#include "Engine/STL.h"

struct StreamLoggerModule : public LoggerModule {
protected:
	std::ostream* streamPtr;
	virtual void InternalProcessMessage(LogOutputData const& outputData) override;

public:
	StreamLoggerModule(std::ostream& stream);
};
