#pragma once
#include "Engine/Logging/LogCategory.h"
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/StandardTypes.h"
#include "Engine/TimeStamp.h"

/** Helper struct to be replaced by std::source_location when it's supported better by compilers */
struct SourceLocation {
public:
	char const* file_name() const { return fileName; }
	uint32_t line() const { return lineNO; }

	SourceLocation() = default;
	SourceLocation(char const* inFileName, uint32_t inLineNO)
	: fileName(inFileName), lineNO(inLineNO)
	{}

private:
	char const* fileName = nullptr;
	uint32_t lineNO = 0;
};

/** Data for a log output operation */
struct LogOutput {
	ClockTimeStamp timeStamp;
	LogCategory const* category;
	ELogVerbosity verbosity;
	SourceLocation location;
	std::string_view message;
};
std::ostream& operator<<(std::ostream& stream, LogOutput const& outputData);
