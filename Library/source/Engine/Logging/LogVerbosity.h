#pragma once
#include <cstdint>

/** The verbosity levels for log output */
enum class ELogVerbosity : uint8_t {
	Debug,
	Message,
	Warning,
	Error,

	//Not used for actual output, used to indicate that no output should be produced.
	Invalid,
};
