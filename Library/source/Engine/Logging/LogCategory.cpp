#include "Engine/Logging/LogCategory.h"
#include "Engine/Ranges.h"

std::deque<LogCategory*> const& LogCategory::GetCategories() {
	return GetMutableCategories();
}

LogCategory* LogCategory::FindCategory(std::string_view name) {
	auto const iter = ranges::find_if(GetCategories(), [&](LogCategory* category) { return category->name.compare(name) == 0; });
	if (iter != GetCategories().end()) return *iter;
	else return nullptr;
}

LogCategory::LogCategory(std::string_view inName, ELogVerbosity inDefaultVerbosity)
: name(inName), defaultVerbosity(inDefaultVerbosity), currentVerbosity(inDefaultVerbosity)
{
	GetMutableCategories().push_back(this);
}

LogCategory::~LogCategory() {
	Algo::RemoveSwap(GetMutableCategories(), this);
}

std::deque<LogCategory*>& LogCategory::GetMutableCategories() {
	static std::deque<LogCategory*> categories;
	return categories;
}

LogCategory LogTemp{ "Temp"sv, ELogVerbosity::Debug };
