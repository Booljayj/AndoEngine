#pragma once
#include <string_view>
#include "Engine/Logging/LogCategory.h"
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/TimeStamp.h"

/** Data for a log output operation */
struct LogOutputData {
	TimeStamp TimeStamp;
	LogCategory const* Category;
	ELogVerbosity Verbosity;
	std::string_view Location;
	std::string_view Message;
};
std::ostream& operator<<( std::ostream& Stream, LogOutputData const& OutputData );
