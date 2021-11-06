#include "Engine/Logging/Logger.h"
#include "Engine/Logging/LogOutputData.h"

Logger Logger::instance;

void Logger::Output(LogCategory const& category, ELogVerbosity verbosity, std::string_view location, std::string_view message) {
	if (verbosity >= category.GetShownVerbosity()) {
		LogOutputData const outputData{ TimeStamp::Now(), &category, verbosity, location, message };

		std::scoped_lock const lock{ mutex };
		for (auto const& modulePair : modules) {
			std::get<1>(modulePair)->ProcessMessage(outputData);
		}
	}
}

void Logger::DestroyModule(size_t id) {
	std::scoped_lock const lock{ mutex };
	auto const existingModuleIterator = std::find_if(modules.begin(), modules.end(), [=](auto const& pair) { return std::get<0>(pair) == id; });
	if (existingModuleIterator != modules.end()) {
		auto const lastModuleIterator = modules.end() - 1;
		std::iter_swap(existingModuleIterator, lastModuleIterator);
		modules.pop_back();
	}
}
