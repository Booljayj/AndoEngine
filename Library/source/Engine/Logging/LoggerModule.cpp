#include <mutex>
#include "Engine/Logging/LoggerModule.h"
#include "Engine/Logging/LogOutputData.h"

void LoggerModule::ProcessMessage( LogOutputData const& OutputData ) {
	std::lock_guard<std::mutex> Guard{ AccessMutex };
	InternalProcessMessage( OutputData );
}
