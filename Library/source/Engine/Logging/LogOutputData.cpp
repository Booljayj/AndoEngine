#include "Engine/Logging/LogCategory.h"
#include "Engine/Logging/LogOutputData.h"
#include "Engine/Logging/LogUtility.h"
#include "Engine/Logging/LogVerbosity.h"

std::ostream& operator<<(std::ostream& stream, LogOutputData const& outputData) {
	//It's very minor, but we're using individual characters instead of string literals to write spaces and parentheses
	//because operator<< needs to try and calculate a string length before putting the characters. That's also the reason
	//why we write string_views instead of character literals, because the length is pre-computed and it's slightly faster.

	stream << outputData.timeStamp << ' ' << outputData.category->GetName() << ' ' << LogUtility::GetText(outputData.verbosity);
#ifdef LOG_INCLUDE_SOURCE_LOCATIONS
	stream << '(' << outputData.location << std::string_view{") "};
#endif
	return stream << outputData.message;
}
