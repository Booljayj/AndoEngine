#include <chrono>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <cstdint>
#include "Engine/Logging/Logger.h"
#include "Engine/Logging/LoggerUtility.h"
#include "Engine/TerminalColors.h"

std::ostream& operator<<( std::ostream& Stream, LogOutputData const& OutputData ) {
	Stream << OutputData.TimeStamp << " " << OutputData.Category->GetName() << " " << LoggerUtility::GetText(OutputData.Verbosity);
#ifdef LOG_INCLUDE_SOURCE_LOCATIONS
	Stream << "(" << OutputData.Location << ") ";
#endif
	return Stream << OutputData.Message;
}

void LoggerModule::ProcessMessage( LogOutputData const& OutputData ) {
	std::lock_guard<std::mutex> Guard{ AccessMutex };
	InternalProcessMessage( OutputData );
}

void Logger::Output( std::string_view Location, LogCategory const& Category, ELogVerbosity Verbosity, std::string_view Message ) const {
	if( Verbosity >= Category.GetShownVerbosity() ) {
		LogOutputData OutputData{ TimeStamp::Now(), &Category,  Verbosity, Location, Message };
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
