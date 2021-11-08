#include "Engine/Time.h"
#include "Engine/Context.h"

TimeController_FixedUpdateVariableRendering::TimeController_FixedUpdateVariableRendering(float inTargetFPS, float inMinFPS) {
	time.unscaledDeltaSeconds = 1.0f/inTargetFPS;
	time.deltaSeconds = time.unscaledDeltaSeconds;
	time.maxDeltaSeconds = 1.0f/inMinFPS;

	time.currentUpdateTimePoint = Time::Clock::now();
}

void TimeController_FixedUpdateVariableRendering::NextFrame() {
	//Swap to the new time point
	time.lastUpdateTimePoint = time.currentUpdateTimePoint;
	time.currentUpdateTimePoint = Time::Clock::now();

	//Calculate the number of seconds that have elapsed since the last frame
	std::chrono::duration<double> const preciseElapsedSeconds = time.currentUpdateTimePoint - time.lastUpdateTimePoint;
	time.elapsedSeconds = std::min(static_cast<float>(preciseElapsedSeconds.count()), time.maxDeltaSeconds);
	time.accumulatedSeconds += time.elapsedSeconds;
}

bool TimeController_FixedUpdateVariableRendering::StartUpdate() {
	//Apply the scale to the deltaSeconds value that will be used this update. We don't allow this to change during an update.
	time.deltaSeconds = time.unscaledDeltaSeconds * timeControl.scale;
	//If the accumulated time is greater than deltaSeconds, enough time has elapsed to do at least one update
	return time.accumulatedSeconds >= time.deltaSeconds;
}

void TimeController_FixedUpdateVariableRendering::FinishUpdate() {
	//Consume the amount of time that was required for this update. If enough accumulatedSeconds
	//remain, we may do another update until this is below the threshold for updates.
	time.accumulatedSeconds -= time.unscaledDeltaSeconds;
	//Advance the time totals now that we have finished an update
	time.unscaledTotalSeconds += time.unscaledDeltaSeconds;
	time.totalSeconds += time.deltaSeconds;
	++time.updateCount;
}
