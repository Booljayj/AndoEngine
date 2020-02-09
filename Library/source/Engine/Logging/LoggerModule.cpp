#include <mutex>
#include "Engine/Logging/LoggerModule.h"
#include "Engine/Logging/LogOutputData.h"

void LoggerModule::ProcessMessage(LogOutputData const& outputData) {
	std::lock_guard<std::mutex> guard{accessMutex};
	InternalProcessMessage(outputData);
}
