#include "Engine/Logging/Logger.h"
#include "Engine/Algo.h"
#include "Engine/Logging/LogOutputDevice.h"

Logger Logger::instance;

void Logger::Output(LogCategory const& category, ELogVerbosity verbosity, SourceLocation location, std::string_view message) {
	if (verbosity <= category.GetMaxVerbosity()) {
		LogOutput const output{ ClockTimeStamp::Now(), &category, verbosity, location, message };

		std::scoped_lock const lock{ mutex };
		for (auto const& devicePair : devices) {
			std::get<1>(devicePair)->ProcessOutput(output);
		}
	}
}

void Logger::Output(LogCategory const& category, ELogVerbosity verbosity, std::string_view message) {
	Output(category, verbosity, SourceLocation{}, message);
}

void Logger::DestroyDevice(size_t id) {
	std::scoped_lock const lock{ mutex };
	Algo::RemoveSwapIf(devices, [=](auto const& pair) { return std::get<0>(pair) == id; });
}
