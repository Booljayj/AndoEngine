#include "Engine/Logging/StreamLoggerModules.h"
#include "Engine/Logging/LogOutputData.h"

StreamLoggerModule::StreamLoggerModule(std::ostream& stream)
: streamPtr(&stream)
{}

void StreamLoggerModule::InternalProcessMessage(LogOutputData const& outputData) {
	*streamPtr << outputData << std::endl;
}
