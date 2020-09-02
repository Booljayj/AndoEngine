#include "Profiling/ProfilerDurationEventScope.h"
#include "Profiling/Profiler.h"

namespace Profiling {
	ProfilerDurationEventScope::ProfilerDurationEventScope(std::string_view inName)
	: name(inName)
	{
		timer.Start();
	}

	ProfilerDurationEventScope::~ProfilerDurationEventScope() {
		Profiler::Get().WriteDurationEvent(name, timer.GetStartTimePoint(), timer.GetCurrentDuration());
	}
}
