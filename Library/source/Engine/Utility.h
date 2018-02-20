//General macro utilities for engine functions

#define STARTUP_SYSTEM( __SYSNAME__ )\
CTX.Log->Message( "Startup "#__SYSNAME__ );\
if( !__SYSNAME__.Startup( CTX ) ) {\
	CTX.Log->Error( "Failed to startup "#__SYSNAME__ );\
	return false;\
}

#define STARTUP_SYSTEM_ARGS( __SYSNAME__, ... )\
CTX.Log->Message( "Startup "#__SYSNAME__ );\
if( !__SYSNAME__.Startup( CTX, __VA_ARGS__ ) ) {\
	CTX.Log->Error( "Failed to startup "#__SYSNAME__ );\
	return false;\
}

#define SHUTDOWN_SYSTEM( __SYSNAME__ )\
CTX.Log->Message( "Shutdown "#__SYSNAME__ );\
if( !__SYSNAME__.Shutdown( CTX ) ) {\
	CTX.Log->Error( "Failed to shutdown "#__SYSNAME__ );\
}

#define TEST_BIT( Mask, Bit ) (Mask & (Bit))
#define SET_BIT( Mask, Bit ) (Mask |= (Bit))
#define CLEAR_BIT( Mask, Bit ) (Mask &= (~Bit))
