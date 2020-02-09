#pragma once
#include "Engine/LogCommands.h"
//General macro utilities for entity-related functions

#define STARTUP_SYSTEM(CAT, SYS_NAME, ...)\
LOG(CAT, Info, "Startup "#SYS_NAME);\
if (!SYS_NAME.Startup(CTX, ##__VA_ARGS__)) {\
	LOG(CAT, Error, "Failed to startup " #SYS_NAME);\
	return false;\
}

#define SHUTDOWN_SYSTEM(CAT, SYS_NAME)\
LOG(CAT, Info, "Shutdown "#SYS_NAME);\
if (!SYS_NAME.Shutdown(CTX)) {\
	LOG(CAT, Error, "Failed to shutdown " #SYS_NAME);\
}
