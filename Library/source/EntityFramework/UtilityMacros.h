#pragma once
#include "Engine/LogCommands.h"
//General macro utilities for entity-related functions

#define STARTUP_SYSTEM( _CAT_, __SYSNAME__, ... )\
LOG( _CAT_, Message, "Startup "#__SYSNAME__ );\
if( !__SYSNAME__.Startup( CTX, ##__VA_ARGS__ ) ) {\
	LOG( _CAT_, Error, "Failed to startup "#__SYSNAME__ );\
	return false;\
}

#define SHUTDOWN_SYSTEM( _CAT_, __SYSNAME__ )\
LOG( _CAT_, Message, "Shutdown "#__SYSNAME__ );\
if( !__SYSNAME__.Shutdown( CTX ) ) {\
	LOG( _CAT_, Error, "Failed to shutdown "#__SYSNAME__ );\
}
