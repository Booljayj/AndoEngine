#include <iostream>
#include "Engine/Logging/Logger.h"

struct StreamLoggerModule : public LoggerModule {
	StreamLoggerModule( std::ostream& Stream );

protected:
	std::ostream* StreamPtr;

	virtual void InternalProcessMessage( LogOutputData const& OutputData ) override;
};
