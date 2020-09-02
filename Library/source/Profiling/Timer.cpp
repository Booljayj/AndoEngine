#include "Profiling/Timer.h"

namespace Profiling {
	void Timer::Start() {
		startTimePoint = Profiling::Now();
	}

	DurationType Timer::GetCurrentDuration() const {
		const TimePointType currentTimePoint = Profiling::Now();
		return currentTimePoint - startTimePoint;
	}
}
