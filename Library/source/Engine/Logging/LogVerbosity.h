#pragma once
#include <cstdint>

/** The verbosity levels for log output */
enum class ELogVerbosity : uint8_t {
	Debug,
	Info,
	Warning,
	Error,
};
