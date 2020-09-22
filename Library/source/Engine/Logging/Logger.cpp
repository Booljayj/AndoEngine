#include "Engine/Logging/Logger.h"
#include "Engine/Logging/LogOutputData.h"

void Logger::Output(LogCategory const& category, ELogVerbosity verbosity, std::string_view location, std::string_view message) const {
	if (verbosity >= category.GetShownVerbosity()) {
		LogOutputData outputData{TimeStamp::Now(), &category, verbosity, location, message};
		for (std::shared_ptr<LoggerModule> const& module : modules) {
			module->ProcessMessage(outputData);
		}
	}
}

void Logger::AddModule(std::shared_ptr<LoggerModule> const& module) {
	auto const existingModuleIterator = std::find(modules.begin(), modules.end(), module);
	if (existingModuleIterator == modules.end()) {
		modules.push_back(module);
	}
}
void Logger::RemoveModule(std::shared_ptr<LoggerModule> const& module) {
	auto const existingModuleIterator = std::find(modules.begin(), modules.end(), module);
	if (existingModuleIterator != modules.end()) {
		auto const lastModuleIterator = modules.end() - 1;
		std::iter_swap(existingModuleIterator, lastModuleIterator);
		modules.pop_back();
	}
}
