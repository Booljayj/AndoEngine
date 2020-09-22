#pragma once
#include "Engine/STL.h"

//Used to hold current time information that can be used by systems
struct Time {
	//Seconds since the last frame
	float deltaSeconds = 1.0f/60.0f;

	//The value of deltaSeconds before scaling is applied
	float unscaledDeltaSeconds = 1.0f/60.0f;
	//Maximum allowed seconds since the last frame
	float maxDeltaSeconds = 1.0f/10.0f;

	//Total seconds since the program started
	float totalSeconds = 0.0f;
	//The value of totalSeconds before scaling is applied
	float unscaledTotalSeconds = 0.0f;

	//Total unused seconds of frame time
	float accumulatedSeconds = 0.0f;
	//Real seconds since the last frame
	float elapsedSeconds = 0.0f;
	//Number of frames that have passed since the program started. Can overflow.
	uint32_t frameCount = 0;

	//Time that the last frame occurred
	std::chrono::time_point<std::chrono::system_clock> lastUpdateTimePoint;
	//Time when this current frame is occurring
	std::chrono::time_point<std::chrono::system_clock> currentUpdateTimePoint;

	/** Get a new value for the current update time */
	void RefreshTime();
	/** Increase the accumulated time by the difference between the last and current time points */
	void IncreaseAccumulatedTime();
	/** Consume DT seconds from the accumulated time */
	void ConsumeAccumulatedTime();
	/** Increase total time by the delta time value */
	void AdvanceDeltaTime();
	/** Apply a scaling factor to delta time */
	void ScaleDeltaTime(float scale);
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
	inline float FrameInterpolationAlpha() const { return time.accumulatedSeconds / time.deltaSeconds; }

	void AdvanceFrame();
	bool StartUpdateFrame();
	void FinishUpdateFrame();
};
