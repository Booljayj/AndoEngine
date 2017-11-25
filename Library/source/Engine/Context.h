#pragma once

#include <cstdint>
#include "Engine/LinearAllocator.h"

/** Macro used to define a function that requires a context. Must appear as the first parameter in the function */
#define CTX_ARG const Context& CTX

/** Macros used to begin and end a block of temporary storage. All temporary memory used inside the block will immediately be recycled. */
#define BEGIN_TEMP_BLOCK const size_t __CTX_LastUsed__ = CTX.Temp.GetUsed()
#define END_TEMP_BLOCK CTX.Temp.SetUsed( __CTX_LastUsed__ )

/** A context is an object that is passed around between many functions, similar to the "this" pointer.
 * It provides a common place to put data that is often considered static or global. It can also expose
 * extremely common utilities, including logging, assertions, and temporary allocations.
 */
struct Context
{
	Context( uint32_t InThreadID, size_t InTempCapacity )
	: ThreadID( InThreadID )
	, Temp( InTempCapacity )
	{}

	/** ID of the thread this context is being used on. Each thread should have its own context */
	uint32_t ThreadID;

	/** function used to print output. */
	void (*Log)( int, const char* );

	/** function called to assert that a condition is true. */
	void (*Assert)( bool, const char* );

	/** linear storage object used for dynamic allocation of small objects */
	LinearAllocatorData Temp;
};
