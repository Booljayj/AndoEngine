#pragma once
#include <string_view>
#include "Profiling/ProfileTypes.h"
#include "Profiling/Timer.h"

namespace Profiling {
	struct ProfilerDurationEventScope {
		std::string_view name;
		Timer timer;

		ProfilerDurationEventScope(std::string_view inName);
		~ProfilerDurationEventScope();
	};
}
