#include <iostream>
#include "Engine/Logging/TerminalLoggerModule.h"
#include "Engine/Logging/LogOutputData.h"
#include "Engine/Logging/LogUtility.h"
#include "Engine/TerminalColors.h"

void TerminalLoggerModule::InternalProcessMessage( LogOutputData const& OutputData ) {
	switch( OutputData.Verbosity ) {
		case ELogVerbosity::Debug:
		std::cout << LogUtility::GetTerminalColor(ELogVerbosity::Debug) << OutputData << TERM_NoColor << std::endl;
		break;

		case ELogVerbosity::Info:
		std::cout << LogUtility::GetTerminalColor(ELogVerbosity::Info) << OutputData << std::endl;
		break;

		case ELogVerbosity::Warning:
		std::cerr << LogUtility::GetTerminalColor(ELogVerbosity::Warning) << OutputData << TERM_NoColor << std::endl;
		break;

		case ELogVerbosity::Error:
		std::cerr << LogUtility::GetTerminalColor(ELogVerbosity::Error) << OutputData << TERM_NoColor << std::endl;
		break;

		default:
		break;
	}
}
