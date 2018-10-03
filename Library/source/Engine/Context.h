#pragma once
#include <cstdint>
#include "Engine/LinearAllocator.h"
#include "Engine/Logger.h"

/** Macro used to define a function that requires a context. Must appear as the first parameter in the function */
#define CTX_ARG Context const& CTX

/** A context is an object that is passed around between many functions, similar to the "this" pointer.
 * It provides a common place to put data that is often considered static or global. It can also expose
 * extremely common utilities, including logging, assertions, and temporary allocations.
 */
struct Context
{
	Context( uint32_t InThreadID, Logger* InLog, size_t InTempCapacity )
		: ThreadID( InThreadID )
		, Log( InLog )
		, Temp( InTempCapacity )
	{}

	/** ID of the thread this context is being used on. Each thread should have its own context */
	uint32_t ThreadID;
	/** Logger object used to print output */
	Logger* Log;
	/** Linear Allocator object used for dynamic allocation of small objects within the thread */
	mutable LinearAllocatorData Temp;
};
