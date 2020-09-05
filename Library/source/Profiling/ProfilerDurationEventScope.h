#pragma once
#include <string_view>
#include "Profiling/ProfileCategory.h"
#include "Profiling/ProfileTypes.h"
#include "Profiling/Timer.h"

namespace Profiling {
	struct ProfilerDurationEventScope {
		std::string_view name;
		const ProfileCategory* category;
		Timer timer;

		ProfilerDurationEventScope(std::string_view inName, const ProfileCategory& inCategory);
		~ProfilerDurationEventScope();
	};
}
