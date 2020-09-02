#pragma once
#include "Profiling/ProfileTypes.h"

namespace Profiling {
	/** A general-purpose high-precision timer */
	struct Timer {
		void Start();

		inline const TimePointType& GetStartTimePoint() const { return startTimePoint; }
		DurationType GetCurrentDuration() const;

	private:
		TimePointType startTimePoint;
	};
}
