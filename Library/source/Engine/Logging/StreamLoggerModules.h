#include <iostream>
#include <mutex>
#include "Engine/Logging/LoggerModule.h"

struct StreamLoggerModule : public LoggerModule {
protected:
	std::ostream* streamPtr;
	virtual void InternalProcessMessage(LogOutputData const& outputData) override;

public:
	StreamLoggerModule(std::ostream& stream);
};
