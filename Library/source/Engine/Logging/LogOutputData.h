#pragma once
#include "Engine/Logging/LogCategory.h"
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/STL.h"
#include "Engine/TimeStamp.h"

/** Data for a log output operation */
struct LogOutputData {
	TimeStamp timeStamp;
	LogCategory const* category;
	ELogVerbosity verbosity;
	std::string_view location;
	std::string_view message;
};
std::ostream& operator<<(std::ostream& stream, LogOutputData const& outputData);
