#pragma once
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/StandardTypes.h"

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
	/** Get the minimum verbosity level which should be shown for this category */
	inline ELogVerbosity GetShownVerbosity() const { return verbosity.load(); }
	/** Modify the minimum verbosity level that should be shown for this category */
	inline void SetShownVerbosity( ELogVerbosity newVerbosity ) { verbosity.store(newVerbosity); }
};

#define DECLARE_LOG_CATEGORY(Name)\
extern LogCategory Log ## Name

#define DEFINE_LOG_CATEGORY(Name, InitialVerbosity)\
LogCategory Log ## Name{ "[" #Name "]", ELogVerbosity::InitialVerbosity }

#define DECLARE_LOG_CATEGORY_MEMBER(Name)\
static LogCategory Log ## Name

#define DEFINE_LOG_CATEGORY_MEMBER(Class, Name, InitialVerbosity)\
LogCategory Class::Log ## Name{ "[" #Name "]", ELogVerbosity::InitialVerbosity }

/** A standard log category that can be used for temporary output */
DECLARE_LOG_CATEGORY(Temp);
