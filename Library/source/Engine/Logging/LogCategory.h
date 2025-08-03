#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/StringView.h"
#include "Engine/Threads.h"

/** A category used to group log output that is related. Output always belongs to a single category */
struct LogCategory {
public:
	/** Get the container that contains all log categories that exist */
	static std::deque<LogCategory*> const& GetCategories();
	/** Find a log category by name */
	static LogCategory* FindCategory(std::string_view name);

	LogCategory(std::string_view inName, ELogVerbosity inDefaultVerbosity);
	~LogCategory();

	/** Get the unique name of this category, which is written to the output. */
	inline std::string_view GetName() const { return name; };

	/** Get the default maximum verbosity level for this category */
	inline ELogVerbosity GetDefaultMaxVerbosity() const { return defaultVerbosity; }
	/** Get the maximum verbosity level which should be shown for this category */
	inline ELogVerbosity GetMaxVerbosity() const { return currentVerbosity.load(); }
	/** Modify the maximum verbosity level that should be shown for this category */
	inline void SetMaxVerbosity(ELogVerbosity newVerbosity) { currentVerbosity.store(newVerbosity); }

private:
	std::string_view name;
	const ELogVerbosity defaultVerbosity;
	std::atomic<ELogVerbosity> currentVerbosity;

	static std::deque<LogCategory*>& GetMutableCategories();
};

/** Standard category for temporary logging */
extern LogCategory LogTemp;
