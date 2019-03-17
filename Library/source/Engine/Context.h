#pragma once
#include <cstdint>
#include <thread>
#include "Engine/LinearAllocator.h"
#include "Engine/Logger.h"

/** Macro used to define a function that requires a context. Must appear as the first parameter in the function */
#define CTX_ARG Context const& CTX

/** A context is an object that is passed around between many functions, similar to the "this" pointer.
 * It provides a common place to put data that is often considered static or global. It can also expose
 * extremely common utilities, including logging, assertions, and temporary allocations.
 */
struct Context {
	Context( Logger* InLog, size_t InTempCapacity )
		: ThreadID( std::this_thread::get_id() )
		, Log( InLog )
		, Temp( InTempCapacity )
	{}

	/** ID of the thread this context is being used on. A context should only be used by a single thread. */
	std::thread::id ThreadID;
	/** Logger object used to print output */
	Logger* Log;
	/** Buffer used for dynamic allocation of small objects within the thread */
	mutable HeapBuffer Temp;
};

/** Standard context macros */
