#pragma once
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

const char* l_printf( CTX_ARG, const char* Format, ... );
l_string l_sprintf( CTX_ARG, const char* Format, ... );

//Describe an object in a way that can be logged as output or used as an input to a formatting operation.
template< class T >
const char* Describe( CTX_ARG, const T& Obj ) { return "[UNKNOWN]"; }

//A macro to simplify logging with Describe, so you don't have to always type the context argument
#define DESC( Obj ) Describe( CTX, Obj )

//Macro for making the specialized describe function a friend, allowing it to access protected members
#define CAN_DESCRIBE( __TYPE__ ) friend const char* Describe( CTX_ARG, const __TYPE__& Value )
//Declare or Define the specialized describe function for a type (use inside the same namespace as the type)
#define DESCRIPTION( __TYPE__ ) const char* Describe( CTX_ARG, const __TYPE__& Value )
