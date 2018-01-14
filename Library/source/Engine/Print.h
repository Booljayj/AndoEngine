#pragma once
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

const char* l_printf( CTX_ARG, const char* Format, ... );
l_string l_sprintf( CTX_ARG, const char* Format, ... );

//Describe an object in a way that can be logged as output or used as an input to a formatting operation.
template< class T >
const char* Describe( CTX_ARG, T& Obj ) { return "[UNKNOWN]"; }

//A macro to simplify logging with Describe, so you don't have to always type the context argument
#define Desc( Obj ) Describe( CTX, Obj )
