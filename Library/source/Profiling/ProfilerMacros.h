#include "Profiling/Profiler.h"
#include "Profiling/ProfilerDurationEventScope.h"

#ifdef DISABLE_PROFILING
#define PROFILE_BEGIN_SESSION(Name)
#define PROFILE_END_SESSION()

#define PROFILE_DURATION(Name)
#define PROFILE_FUNCTION()
#define PROFILE_INSTANT(Name)
#define PROFILE_COUNTER(Name, Value)

#else
#define PROFILE_BEGIN_SESSION(Name) ::Profiling::Profiler::Get().BeginSession(CTX, Name)
#define PROFILE_END_SESSION() ::Profiling::Profiler::Get().EndSession()

#define PROFILE_DURATION(Name) const ::Profiling::ProfilerDurationEventScope scope_ ## __COUNTER__{ Name }
#define PROFILE_FUNCTION() PROFILE_DURATION(__FUNCTION__)
#define PROFILE_INSTANT(Name) ::Profiling::Profiler::Get().WriteInstantEvent(Name, ::Profiling::Now())
#define PROFILE_COUNTER(Name, Value) ::Profiling::Profiler::Get().WriteCounterEvent(Name, ::Profiling::Now(), Value)

#endif
