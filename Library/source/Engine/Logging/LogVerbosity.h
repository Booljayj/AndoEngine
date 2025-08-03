#pragma once
#include "Engine/Core.h"

/** The verbosity levels for log output */
enum class ELogVerbosity : uint8_t {
	/** Unexpected states or behaviors that should be addressed and can cause instability */
	Error,
	/** Unexpected states or behaviors that should be addressed */
	Warning,
	/** Information about general program processes */
	Info,
	/** Internal system dumps and low-level tracing */
	Debug,
};
