#include "Engine/Time.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

void Time::RefreshTime() {
	lastUpdateTimePoint = currentUpdateTimePoint;
	currentUpdateTimePoint = std::chrono::system_clock::now();
}

void Time::IncreaseAccumulatedTime() {
	std::chrono::duration<double> const preciseElapsedSeconds = currentUpdateTimePoint - lastUpdateTimePoint;
	elapsedSeconds = std::min(static_cast<float>(preciseElapsedSeconds.count()), maxDeltaSeconds);
	accumulatedSeconds += elapsedSeconds;
}

void Time::ConsumeAccumulatedTime() {
	accumulatedSeconds -= deltaSeconds;
}

void Time::AdvanceDeltaTime() {
	unscaledTotalSeconds += unscaledDeltaSeconds;
	totalSeconds += deltaSeconds;
}

void Time::ScaleDeltaTime(float scale) {
	deltaSeconds = unscaledDeltaSeconds * scale;
}

TimeController_FixedUpdateVariableRendering::TimeController_FixedUpdateVariableRendering(float inTargetFPS, float inMinFPS) {
	time.unscaledDeltaSeconds = 1.0f/inTargetFPS;
	time.deltaSeconds = time.unscaledDeltaSeconds;
	time.maxDeltaSeconds = 1.0f/inMinFPS;
	time.RefreshTime();
}

void TimeController_FixedUpdateVariableRendering::AdvanceFrame() {
	++time.frameCount;
	time.RefreshTime();
	time.IncreaseAccumulatedTime();
}

bool TimeController_FixedUpdateVariableRendering::StartUpdateFrame() {
	time.ScaleDeltaTime(timeControl.scale);
	time.AdvanceDeltaTime();
	return time.accumulatedSeconds >= time.deltaSeconds;
}

void TimeController_FixedUpdateVariableRendering::FinishUpdateFrame() {
	time.ConsumeAccumulatedTime();
}
