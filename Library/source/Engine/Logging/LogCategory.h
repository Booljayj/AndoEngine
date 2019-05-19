#pragma once
#include <atomic>
#include "Engine/Logging/LogVerbosity.h"

/** A category used to group log output that is related. Output always belongs to a single category */
struct LogCategory {
private:
	char const* Name;
	std::atomic<ELogVerbosity> Verbosity;

public:
	LogCategory( char const* InName, ELogVerbosity InVerbosity )
	: Name( InName ), Verbosity( InVerbosity )
	{}

	inline char const* GetName() const { return Name; };
	/** True if the output with the specified verbosity should be shown right now for this category */
	inline ELogVerbosity GetShownVerbosity() const { return Verbosity.load(); }
	/** Modify the verbosity level that is allow for output that belongs to this category */
	inline void SetShownVerbosity( ELogVerbosity NewVerbosity ) { Verbosity.store( NewVerbosity ); }
};

#define DECLARE_LOG_CATEGORY( _NAME_ )\
extern LogCategory Log ## _NAME_

#define DEFINE_LOG_CATEGORY( _NAME_, _INITIAL_VERBOSITY_ )\
LogCategory Log ## _NAME_{ "[" #_NAME_ "]", ELogVerbosity::_INITIAL_VERBOSITY_ }

DECLARE_LOG_CATEGORY( Temp );
