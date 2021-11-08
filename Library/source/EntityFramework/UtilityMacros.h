#pragma once
#include "Engine/LogCommands.h"
//General macro utilities for entity-related functions

#define STARTUP_SYSTEM(Category, SystemName, ...)\
LOG(Category, Info, "Startup "#SystemName);\
if (!SystemName.Startup(__VA_ARGS__)) {\
	LOG(Category, Error, "Failed to startup " #SystemName);\
	return false;\
}

#define SHUTDOWN_SYSTEM(Category, SystemName, ...)\
LOG(Category, Info, "Shutdown "#SystemName);\
if (!SystemName.Shutdown(__VA_ARGS__)) {\
	LOG(Category, Error, "Failed to shutdown " #SystemName);\
}
