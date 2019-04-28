#include <iostream>
#include "Engine/Logging/TerminalLoggerModule.h"
#include "Engine/TerminalColors.h"

void TerminalLoggerModule::InternalProcessMessage( LogOutputData const& OutputData ) {
	switch( OutputData.Verbosity ) {
		case ELogVerbosity::Debug:
		std::cout << TERM_Cyan << OutputData.Category->GetName() << ": " << OutputData.Message << TERM_NoColor << std::endl;
		break;

		case ELogVerbosity::Message:
		std::cout << TERM_NoColor << OutputData.Category->GetName() << ": " << OutputData.Message << std::endl;
		break;

		case ELogVerbosity::Warning:
		std::cerr << TERM_Yellow << OutputData.Category->GetName() << ": [W] " << OutputData.Message << TERM_NoColor << std::endl;
		break;

		case ELogVerbosity::Error:
		std::cerr << TERM_Red << OutputData.Category->GetName() << ": [E] " << OutputData.Message << TERM_NoColor << std::endl;
		break;

		default:
		break;
	}
}
