#pragma once
#include <atomic>
#include <string_view>
#include "Engine/Logging/LogVerbosity.h"

/** A category used to group log output that is related. Output always belongs to a single category */
struct LogCategory {
private:
	std::string_view name;
	std::atomic<ELogVerbosity> verbosity;

public:
	LogCategory(std::string_view inName, ELogVerbosity inVerbosity)
	: name(inName), verbosity(inVerbosity)
	{}

	inline std::string_view GetName() const { return name; };
	/** True if the output with the specified verbosity should be shown right now for this category */
	inline ELogVerbosity GetShownVerbosity() const { return verbosity.load(); }
	/** Modify the verbosity level that is allow for output that belongs to this category */
	inline void SetShownVerbosity( ELogVerbosity newVerbosity ) { verbosity.store( newVerbosity ); }
};

#define DECLARE_LOG_CATEGORY(Name)\
extern LogCategory Log ## Name

#define DEFINE_LOG_CATEGORY(Name, InitialVerbosity)\
LogCategory Log ## Name{ "[" #Name "]", ELogVerbosity::InitialVerbosity }

DECLARE_LOG_CATEGORY(Temp);
