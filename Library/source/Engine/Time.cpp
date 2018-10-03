#include "Engine/Time.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

void Time::RefreshTime()
{
	LastUpdateTime = CurrentUpdateTime;
	CurrentUpdateTime = std::chrono::system_clock::now();
}

void Time::IncreaseAccumulatedTime()
{
	std::chrono::duration<double> const PreciseElapsedTime = CurrentUpdateTime - LastUpdateTime;
	ElapsedTime = std::min( static_cast<float>( PreciseElapsedTime.count() ), MaxDeltaTime );
	AccumulatedTime += ElapsedTime;
}

void Time::ConsumeAccumulatedTime()
{
	AccumulatedTime -= DeltaTime;
}

void Time::AdvanceDeltaTime()
{
	UnscaledTotalTime += UnscaledDeltaTime;
	TotalTime += DeltaTime;
}

void Time::ScaleDeltaTime( float Scale )
{
	DeltaTime = UnscaledDeltaTime * Scale;
}

TimeController_FixedUpdateVariableRendering::TimeController_FixedUpdateVariableRendering( float InTargetFPS, float InMinFPS )
{
	T.UnscaledDeltaTime = 1.0f/InTargetFPS;
	T.DeltaTime = T.UnscaledDeltaTime;
	T.MaxDeltaTime = 1.0f/InMinFPS;
	T.RefreshTime();
}

void TimeController_FixedUpdateVariableRendering::AdvanceFrame()
{
	++T.FrameCount;
	T.RefreshTime();
	T.IncreaseAccumulatedTime();
}

bool TimeController_FixedUpdateVariableRendering::StartUpdateFrame()
{
	T.ScaleDeltaTime( TControl.Scale );
	T.AdvanceDeltaTime();
	return T.AccumulatedTime >= T.DeltaTime;
}

void TimeController_FixedUpdateVariableRendering::FinishUpdateFrame()
{
	T.ConsumeAccumulatedTime();
}
