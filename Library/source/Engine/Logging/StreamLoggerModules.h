#include <iostream>
#include <mutex>
#include "Engine/Logging/LoggerModule.h"

struct StreamLoggerModule : public LoggerModule {
protected:
	std::ostream* StreamPtr;
	virtual void InternalProcessMessage( LogOutputData const& OutputData ) override;

public:
	StreamLoggerModule( std::ostream& Stream );
};
