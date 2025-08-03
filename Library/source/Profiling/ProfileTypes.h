#pragma once
#include "Engine/Core.h"

namespace Profiling {
	using DurationType = std::chrono::duration<uint64_t, std::micro>;
	using TimePointType = std::chrono::time_point<std::chrono::steady_clock, DurationType>;

	inline TimePointType Now() noexcept { return std::chrono::time_point_cast<DurationType>(TimePointType::clock::now()); }
}
