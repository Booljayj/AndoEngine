#include "Engine/Logging/FileLoggerModule.h"
#include "Engine/Logging/LogOutputData.h"

FileLoggerModule::FileLoggerModule(std::string_view fileName)
: stream(fileName.data(), std::ios::out | std::ios::app)
{}

void FileLoggerModule::ProcessMessage(LogOutputData const& outputData) {
	if (stream.is_open() && stream.good()) {
		stream << outputData << std::endl;
	}
}
