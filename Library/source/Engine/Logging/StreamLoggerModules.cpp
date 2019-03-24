#include "Engine/Logging/StreamLoggerModules.h"

StreamLoggerModule::StreamLoggerModule( std::ostream& Stream )
: StreamPtr( &Stream )
{}

void StreamLoggerModule::InternalProcessMessage( LogOutputData const& OutputData ) {
	std::ostream& Stream = *StreamPtr;
	switch( OutputData.Verbosity ) {
		case ELogVerbosity::Debug:
		Stream << OutputData.Category->GetName() << ": " << OutputData.Message << std::endl;
		break;

		case ELogVerbosity::Message:
		Stream << OutputData.Category->GetName() << ": " << OutputData.Message << std::endl;
		break;

		case ELogVerbosity::Warning:
		Stream << OutputData.Category->GetName() << ": [W] " << OutputData.Message << std::endl;
		break;

		case ELogVerbosity::Error:
		Stream << OutputData.Category->GetName() << ": [E] " << OutputData.Message << std::endl;
		break;

		default:
		break;
	}
}
