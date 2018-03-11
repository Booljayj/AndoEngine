#pragma once
#include <chrono>
#include "Engine/Print.h"

//Used to hold current time information that can be used by systems
struct Time
{
	//Seconds since the last frame
	float DeltaTime = 1.0f/60.0f;

	//The value of DeltaTime before scaling is applied
	float UnscaledDeltaTime = 1.0f/60.0f;
	//Maximum allowed seconds since the last frame
	float MaxDeltaTime = 1.0f/10.0f;

	//Total seconds since the program started
	float TotalTime = 0.0f;
	//The value of TotalTime before scaling is applied
	float UnscaledTotalTime = 0.0f;

	//Total unused seconds of frame time
	float AccumulatedTime = 0.0f;
	//Real seconds since the last frame
	float ElapsedTime = 0.0f;
	//Number of frames that have passed since the program started. Can overflow.
	uint32_t FrameCount = 0;

	//Time that the last frame occurred
	std::chrono::time_point<std::chrono::system_clock> LastUpdateTime;
	//Time when this current frame is occurring
	std::chrono::time_point<std::chrono::system_clock> CurrentUpdateTime;

	/** Get a new value for the current update time */
	void RefreshTime();
	/** Increase the accumulated time by the difference between the last and current time points */
	void IncreaseAccumulatedTime();
	/** Consume DT seconds from the accumulated time */
	void ConsumeAccumulatedTime();
	/** Increase total time by the delta time value */
	void AdvanceDeltaTime();
	/** Apply a scaling factor to delta time */
	void ScaleDeltaTime( float Scale );
};

//Used to control the behavior of a time controller
struct TimeControl
{
	//The scaling factor applied to DeltaTime
	float Scale = 1.0f;
};

//@todo Implement a few other TimeController types, just for the heck of it.
//@todo Move TimeController implementations into separate files to keep things more segmented and clean.

struct TimeController_FixedUpdateVariableRendering
{
	CAN_DESCRIBE( TimeController_FixedUpdateVariableRendering );

private:
	Time T;
	TimeControl TControl;

public:
	TimeController_FixedUpdateVariableRendering( float InTargetFPS, float InMinFPS );

	inline Time const& GetTime() const { return T; }
	inline TimeControl& GetTimeControl() { return TControl; }
	inline float FrameInterpolationAlpha() const { return T.AccumulatedTime / T.DeltaTime; }

	void AdvanceFrame();
	bool StartUpdateFrame();
	void FinishUpdateFrame();
};

DESCRIPTION( TimeController_FixedUpdateVariableRendering );
