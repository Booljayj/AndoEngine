#include <fstream>
#include "Engine/Logging/FileLoggerModule.h"

FileLoggerModule::FileLoggerModule( std::string_view FileName )
: Stream( FileName.data(), std::ios::out | std::ios::app )
{}

void FileLoggerModule::InternalProcessMessage( LogOutputData const& OutputData ) {
	if( Stream.is_open() && Stream.good() ) {
		Stream << OutputData << std::endl;
	}
}
