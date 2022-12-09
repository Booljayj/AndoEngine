#include "Engine/Logging/LogOutputDevice.h"
#include "Engine/Logging/LogOutput.h"
#include "Engine/Logging/LogUtility.h"
#include "Engine/StandardTypes.h"
#include "Engine/TerminalColors.h"

#if defined(_MSC_VER)
#include "Windows.h"
#endif

TerminalOutputDevice::TerminalOutputDevice() {
#if defined(_MSC_VER)
	string << "Starting Log at " << TimeStamp::Now() << std::endl;
	::OutputDebugStringA(string.str().c_str());
	string.str(std::string{});
	string.clear();
#else
	std::cout << "Starting Log at " << TimeStamp::Now() << std::endl;
#endif
}

void TerminalOutputDevice::ProcessOutput(LogOutput const& output) {
#if defined(_MSC_VER)
	string << output << std::endl;
	::OutputDebugStringA(string.str().c_str());
	string.str(std::string{});
	string.clear();
#else
	switch (output.verbosity) {
		case ELogVerbosity::Debug:
		std::cout << LogUtility::GetTerminalColor(ELogVerbosity::Debug) << output << TERM_NoColor << std::endl;
		break;

		case ELogVerbosity::Info:
		std::cout << LogUtility::GetTerminalColor(ELogVerbosity::Info) << output << std::endl;
		break;

		case ELogVerbosity::Warning:
		std::cerr << LogUtility::GetTerminalColor(ELogVerbosity::Warning) << output << TERM_NoColor << std::endl;
		break;

		case ELogVerbosity::Error:
		std::cerr << LogUtility::GetTerminalColor(ELogVerbosity::Error) << output << TERM_NoColor << std::endl;
		break;

		default:
		break;
	}
#endif
}

StreamOutputDevice::StreamOutputDevice(std::ostream& inStream)
: stream(inStream)
{
	std::cout << "Starting Log at " << TimeStamp::Now() << std::endl;
}

void StreamOutputDevice::ProcessOutput(LogOutput const& output) {
	stream << output << std::endl;
}

FileOutputDevice::FileOutputDevice(std::string_view fileName)
: stream(fileName.data(), std::ios::out | std::ios::app)
{
	if (stream.is_open() && stream.good()) {
		stream << "Starting Log at " << TimeStamp::Now() << std::endl;
	}
}

void FileOutputDevice::ProcessOutput(LogOutput const& output) {
	if (stream.is_open() && stream.good()) {
		stream << output << std::endl;
	}
}
