#pragma once
#include "UtilityMacros.h"

//Describe an object in a way that can be logged as output or used as an input to a formatting operation.
template< class T >
char const* Describe( CTX_ARG, T const& Obj ) { return "[UNKNOWN]"; }

//A macro to simplify logging with Describe, so you don't have to always type the context argument
#define DESC( Obj ) Describe( CTX, Obj )

//Declare or Define the specialized describe function for a type (use inside the same namespace as the type)
#define DESCRIPTION( __TYPE__ ) char const* Describe( CTX_ARG, __TYPE__ const& Value )
//Macro for making the specialized describe function a friend, allowing it to access non-public members
#define CAN_DESCRIBE( __TYPE__ ) friend DESCRIPTION( __TYPE__ )
