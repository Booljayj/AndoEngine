#include "Engine/Logging/StreamLoggerModules.h"
#include "Engine/Logging/LogOutputData.h"

StreamLoggerModule::StreamLoggerModule( std::ostream& Stream )
: StreamPtr( &Stream )
{}

void StreamLoggerModule::InternalProcessMessage( LogOutputData const& OutputData ) {
	*StreamPtr << OutputData << std::endl;
}
