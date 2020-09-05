#include "Profiling/ProfilerDurationEventScope.h"
#include "Profiling/Profiler.h"

namespace Profiling {
	ProfilerDurationEventScope::ProfilerDurationEventScope(std::string_view inName, const ProfileCategory& inCategory)
	: name(inName), category(&inCategory)
	{
		timer.Start();
	}

	ProfilerDurationEventScope::~ProfilerDurationEventScope() {
		Profiler::Get().WriteDurationEvent(name, *category, timer.GetStartTimePoint(), timer.GetCurrentDuration());
	}
}
