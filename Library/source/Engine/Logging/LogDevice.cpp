#include "Engine/Logging/LogDevice.h"
#include "Engine/Logging/LogMessage.h"
#include "Engine/Logging/LogUtility.h"
#include "Engine/StandardTypes.h"
#include "Engine/TerminalColors.h"

#if defined(_MSC_VER)
#include "Windows.h"
#endif

ILogDeviceCollection& operator<<(ILogDeviceCollection& devices, LogMessageQueue& queue) {
	//Allow the devices to process the log messages
	//We'll go through each device first because they're not inline, while the messages are.
	for (auto const& device : devices) {
		for (LogMessage const& entry : queue) device->ProcessMessage(entry.header, entry.message);
	}

	queue.Reset();

	return devices;
}

TerminalLogDevice::TerminalLogDevice() {
#if defined(_MSC_VER)
	buffer.reserve(512);
	std::format_to(std::back_inserter(buffer), "Starting log at {0}\n", TimeStamp::Now());
	::OutputDebugStringA(buffer.c_str());
	buffer.clear();
#else
	std::format_to(std::ostream_iterator<char>(std::cout), "Starting log at {0}\n", TimeStamp::Now());
#endif
}

void TerminalLogDevice::ProcessMessage(LogMessageHeader const& header, std::string const& message) noexcept {
	auto const& time = header.time;
	std::string_view const category = header.category->GetName();
	std::string_view const verbosity = LogUtility::GetText(header.verbosity);
	char const* file = header.location.file_name();
	size_t const line = header.location.line();

#if defined(_MSC_VER)
	std::format_to(std::back_inserter(buffer), "{0} [{1}] {2}: {3} ({4}:{5})\n", time, category, verbosity, message, file, line);
	::OutputDebugStringA(buffer.c_str());
	buffer.clear();
#else
	std::string_view const color = LogUtility::GetTerminalColor(header.verbosity);
	std::format_to(std::ostream_iterator<char>(std::cout), "{0}{1} [{2}] {3}: {4} ({5}:{6})" TERM_NoColor "\n", color, time, category, verbosity, message, file, line);
#endif
}

StreamLogDevice::StreamLogDevice(std::ostream& stream)
	: stream(stream)
{
	if (stream.good()) {
		std::format_to(std::ostream_iterator<char>(stream), "Starting log at {0}\n", TimeStamp::Now());
	}
}

void StreamLogDevice::ProcessMessage(LogMessageHeader const& header, std::string const& message) noexcept {
	if (stream.good()) {
		auto const& time = header.time;
		std::string_view const category = header.category->GetName();
		std::string_view const verbosity = LogUtility::GetText(header.verbosity);
		char const* file = header.location.file_name();
		size_t const line = header.location.line();

		std::format_to(std::ostream_iterator<char>(stream), "{0} [{1}] {2}: {3} ({4}:{5})\n", time, category, verbosity, message, file, line);
		stream.flush();
	}
}

FileLogDevice::FileLogDevice(std::string_view fileName)
	: stream(fileName.data(), std::ios::out | std::ios::app)
{
	if (stream.good()) {
		std::format_to(std::ostream_iterator<char>(stream), "Starting log at {0}\n", TimeStamp::Now());
		stream.flush();
	}
}

void FileLogDevice::ProcessMessage(LogMessageHeader const& header, std::string const& message) noexcept {
	if (stream.good()) {
		auto const& time = header.time;
		std::string_view const category = header.category->GetName();
		std::string_view const verbosity = LogUtility::GetText(header.verbosity);
		char const* file = header.location.file_name();
		size_t const line = header.location.line();

		std::format_to(std::ostream_iterator<char>(stream), "{0} [{1}] {2}: {3} ({4}:{5})\n", time, category, verbosity, message, file, line);
		stream.flush();
	}
}
