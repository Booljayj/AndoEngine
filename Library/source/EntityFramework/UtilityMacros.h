#pragma once
//General macro utilities for entity-related functions

#define STARTUP_SYSTEM( __SYSNAME__, ... )\
CTX.Log->Message( "Startup "#__SYSNAME__ );\
if( !__SYSNAME__.Startup( CTX, ##__VA_ARGS__ ) ) {\
	CTX.Log->Error( "Failed to startup "#__SYSNAME__ );\
	return false;\
}

#define SHUTDOWN_SYSTEM( __SYSNAME__ )\
CTX.Log->Message( "Shutdown "#__SYSNAME__ );\
if( !__SYSNAME__.Shutdown( CTX ) ) {\
	CTX.Log->Error( "Failed to shutdown "#__SYSNAME__ );\
}