#include "Engine/Logging/LogCategory.h"
#include "Engine/Logging/LogOutputData.h"
#include "Engine/Logging/LogUtility.h"
#include "Engine/Logging/LogVerbosity.h"

std::ostream& operator<<( std::ostream& Stream, LogOutputData const& OutputData ) {
	Stream << OutputData.TimeStamp << " " << OutputData.Category->GetName() << " " << LogUtility::GetText(OutputData.Verbosity);
#ifdef LOG_INCLUDE_SOURCE_LOCATIONS
	Stream << "(" << OutputData.Location << ") ";
#endif
	return Stream << OutputData.Message;
}
