#pragma once
#include <mutex>
#include "Engine/Logging/LogVerbosity.h"

/** A category used to group log output that is related. Output always blongs to a single category */
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
	inline ELogVerbosity ShownVerbosity() const { return Verbosity.load(); }
	/** Modify the verbosity level that is allow for output that belongs to this category */
	inline void SetShownVerbosity( ELogVerbosity NewVerbosity ) { Verbosity.store( NewVerbosity ); }
};

#define DECLARE_LOG_CATEGORY( _NAME_, _INITIAL_VERBOSITY_ )\
struct LogCategory_ ## _NAME_ : public LogCategory {\
	LogCategory_ ## _NAME_() : LogCategory( #_NAME_, ELogVerbosity::_INITIAL_VERBOSITY_ ) {};\
};\
extern LogCategory_ ## _NAME_ const _NAME_

#define DEFINE_LOG_CATEGORY( _NAME_ )\
LogCategory_ ## _NAME_ const _NAME_{}

#define DEFINE_LOG_CATEGORY_STATIC( _NAME_, _INITIAL_VERBOSITY_ )\
struct LogCategory_ ## _NAME_ : public LogCategory {\
	LogCategory_ ## _NAME_() : LogCategory( #_NAME_, ELogVerbosity::_INITIAL_VERBOSITY_ ) {};\
};\
LogCategory_ ## _NAME_ const _NAME_{}

DECLARE_LOG_CATEGORY( LogTemp, Debug );
