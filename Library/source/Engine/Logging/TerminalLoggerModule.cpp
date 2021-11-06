#include "Engine/Logging/TerminalLoggerModule.h"
#include "Engine/Logging/LogOutputData.h"
#include "Engine/Logging/LogUtility.h"
#include "Engine/STL.h"
#include "Engine/TerminalColors.h"

void TerminalLoggerModule::ProcessMessage(LogOutputData const& outputData) {
	switch (outputData.verbosity) {
		case ELogVerbosity::Debug:
		std::cout << LogUtility::GetTerminalColor(ELogVerbosity::Debug) << outputData << TERM_NoColor << std::endl;
		break;

		case ELogVerbosity::Info:
		std::cout << LogUtility::GetTerminalColor(ELogVerbosity::Info) << outputData << std::endl;
		break;

		case ELogVerbosity::Warning:
		std::cerr << LogUtility::GetTerminalColor(ELogVerbosity::Warning) << outputData << TERM_NoColor << std::endl;
		break;

		case ELogVerbosity::Error:
		std::cerr << LogUtility::GetTerminalColor(ELogVerbosity::Error) << outputData << TERM_NoColor << std::endl;
		break;

		default:
		break;
	}
}
