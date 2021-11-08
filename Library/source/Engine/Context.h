#pragma once
#include "Engine/Logging/Logger.h"
#include "Engine/STL.h"

/** Macro used to define a function that requires a context. Must appear as the first parameter in the function */
#define CTX_ARG Context& CTX

/** A context is an object that is passed around between many functions, similar to the "this" pointer.
 * It provides a common place to put data that is often considered static or global. It can also expose
 * extremely common utilities, including logging, assertions, and temporary allocations.
 */
struct Context {};
