#include <iostream>
#include "Engine/Logging/Logger.h"
#include "Engine/TerminalColors.h"

void LoggerModule::ProcessMessage( LogOutputData const& OutputData ) {
	AccessMutex.lock();
	InternalProcessMessage( OutputData );
	AccessMutex.unlock();
}

void Logger::Output( std::string_view Location, LogCategory const& Category, ELogVerbosity Verbosity, std::string_view Message ) const {
	if( Verbosity >= Category.ShownVerbosity() ) {
		LogOutputData OutputData{ Location, &Category, Verbosity, Message };
		for( std::shared_ptr<LoggerModule> const& Module : Modules ) {
			Module->ProcessMessage( OutputData );
		}
	}
}

void Logger::AddModule( std::shared_ptr<LoggerModule> const& Module ) {
	auto const ExistingModuleIterator = std::find( Modules.begin(), Modules.end(), Module );
	if( ExistingModuleIterator == Modules.end() ) {
		Modules.push_back( Module );
	}
}
void Logger::RemoveModule( std::shared_ptr<LoggerModule> const& Module ) {
	auto const ExistingModuleIterator = std::find( Modules.begin(), Modules.end(), Module );
	if( ExistingModuleIterator != Modules.end() ) {
		auto const LastModuleIterator = Modules.end() - 1;
		std::iter_swap( ExistingModuleIterator, LastModuleIterator );
		Modules.pop_back();
	}
}

void StandardOutputLoggerModule::InternalProcessMessage( LogOutputData const& OutputData ) {
	switch( OutputData.Verbosity ) {
		case ELogVerbosity::Debug:
		TerminalOutput( TERM_Cyan, "", OutputData );
		break;

		case ELogVerbosity::Message:
		TerminalOutput( TERM_NoColor, "", OutputData );
		break;

		case ELogVerbosity::Warning:
		TerminalOutput( TERM_Yellow, "[W] ", OutputData );
		break;

		case ELogVerbosity::Error:
		TerminalOutput( TERM_Red, "[E] ", OutputData );
		break;

		default:
		break;
	}
}

void StandardOutputLoggerModule::TerminalOutput( char const* ColorCode, char const* Prefix, LogOutputData const& OutputData  ) {
	std::cout << ColorCode << OutputData.Category->GetName() << ": " << Prefix << OutputData.Message << TERM_NoColor << std::endl;
}
