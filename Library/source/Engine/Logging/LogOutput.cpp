#include "Engine/Logging/LogOutput.h"
#include "Engine/Logging/LogCategory.h"
#include "Engine/Logging/LogConfig.h"
#include "Engine/Logging/LogUtility.h"
#include "Engine/Logging/LogVerbosity.h"

std::ostream& operator<<(std::ostream& stream, LogOutput const& output) {
	stream << output.timeStamp << " ["sv << output.category->GetName() << "] "sv << LogUtility::GetText(output.verbosity);
	if constexpr(LogConfig::IncludeLocations()) {
		stream << '(' << output.location.file_name() << ":" << output.location.line() << ") "sv;
	}
	return stream << output.message;
}
