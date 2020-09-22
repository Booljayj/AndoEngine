#pragma once
#include "Profiling/Profiler.h"
#include "Profiling/ProfilerDurationEventScope.h"

#ifdef DISABLE_PROFILING
#define PROFILE_BEGIN_SESSION(Name)
#define PROFILE_END_SESSION()

#define PROFILE_DURATION(Name, Category)
#define PROFILE_FUNCTION(Category)
#define PROFILE_INSTANT(Name, Category)
#define PROFILE_COUNTER(Name, Category, Value)

#else
#define PROFILE_BEGIN_SESSION(Name) ::Profiling::Profiler::Get().BeginSession(CTX, Name)
#define PROFILE_END_SESSION() ::Profiling::Profiler::Get().EndSession()

#define PROFILE_DURATION(Name, Category) const ::Profiling::ProfilerDurationEventScope scope_ ## __COUNTER__{ Name, Profile ## Category }
#define PROFILE_FUNCTION(Category) PROFILE_DURATION(__FUNCTION__, Category)
#define PROFILE_INSTANT(Name, Category) ::Profiling::Profiler::Get().WriteInstantEvent(Name, Profile ## Category, ::Profiling::Now())
#define PROFILE_COUNTER(Name, Category, Value) ::Profiling::Profiler::Get().WriteCounterEvent(Name, Profile ## Category, ::Profiling::Now(), Value)

#endif
