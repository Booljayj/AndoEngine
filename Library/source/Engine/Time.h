#pragma once
#include "Engine/Core.h"

/**
 * Used to hold current time information that can be used by systems.
 * Refresh: when the time values are updated using the high-precision time from the system clock, recalculating the time since the last refresh
 * Update: a process that occurs with a period of unscaledDeltaSeconds.
 * Frame: a process that occurs every time a refresh happens.
 */
struct Time {
	using Clock = std::chrono::system_clock;
	using TimePoint = std::chrono::time_point<Clock>;

	/** Seconds since the last fixed update */
	float deltaSeconds = 1.0f/60.0f;

	/** The value of deltaSeconds before scaling is applied */
	float unscaledDeltaSeconds = 1.0f/60.0f;
	/** Maximum allowed seconds since the last frame */
	float maxDeltaSeconds = 1.0f/10.0f;

	/** Total seconds since the time tracking started */
	float totalSeconds = 0.0f;
	/** The value of totalSeconds before scaling is applied */
	float unscaledTotalSeconds = 0.0f;

	/** Total amount of real seconds that have passed since the last update */
	float accumulatedSeconds = 0.0f;
	/** Total amount of real seconds that have passed since the last frame */
	float elapsedSeconds = 0.0f;
	/** Number of updates that have happened since the program started. Can overflow. */
	uint32_t updateCount = 0;

	/** Time that the last frame occurred */
	TimePoint lastUpdateTimePoint;
	/** Time when this current frame is occurring */
	TimePoint currentUpdateTimePoint;
};

//Used to control the behavior of a time controller
struct TimeControl {
	//The scaling factor applied to deltaSeconds
	float scale = 1.0f;
};

//@todo Implement a few other TimeController types, just for the heck of it.
//@todo Move TimeController implementations into separate files to keep things more segmented and clean.

struct TimeController_FixedUpdateVariableRendering {
private:
	Time time;
	TimeControl timeControl;

public:
	TimeController_FixedUpdateVariableRendering(float inTargetFPS, float inMinFPS);

	inline Time const& GetTime() const { return time; }
	inline TimeControl& GetTimeControl() { return timeControl; }

	/** The amount of progress from the previous update to the next update, expressed as a ratio from previous ot next */
	inline float Alpha() const { return time.accumulatedSeconds / time.unscaledDeltaSeconds; }

	/** Advance to the next frame, recording how much real time has passed. */
	void NextFrame();
	/** Attempt to start a fixed update for the target framerate. Returns true if enough time has passed to do a fixed update. */
	bool StartUpdate();
	/** Finish a fixed update for the target framerate */
	void FinishUpdate();
};
