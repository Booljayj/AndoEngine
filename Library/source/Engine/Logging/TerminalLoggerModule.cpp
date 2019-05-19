#include <iostream>
#include "Engine/Logging/TerminalLoggerModule.h"
#include "Engine/Logging/LoggerUtility.h"
#include "Engine/TerminalColors.h"

void TerminalLoggerModule::InternalProcessMessage( LogOutputData const& OutputData ) {
	switch( OutputData.Verbosity ) {
		case ELogVerbosity::Debug:
		std::cout << LoggerUtility::GetTerminalColor(ELogVerbosity::Debug) << OutputData << TERM_NoColor << std::endl;
		break;

		case ELogVerbosity::Info:
		std::cout << LoggerUtility::GetTerminalColor(ELogVerbosity::Info) << OutputData << std::endl;
		break;

		case ELogVerbosity::Warning:
		std::cerr << LoggerUtility::GetTerminalColor(ELogVerbosity::Warning) << OutputData << TERM_NoColor << std::endl;
		break;

		case ELogVerbosity::Error:
		std::cerr << LoggerUtility::GetTerminalColor(ELogVerbosity::Error) << OutputData << TERM_NoColor << std::endl;
		break;

		default:
		break;
	}
}
