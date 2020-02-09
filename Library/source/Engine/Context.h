#pragma once
#include <cstdint>
#include <thread>
#include "Engine/LinearAllocator.h"
#include "Engine/Logging/Logger.h"

/** Macro used to define a function that requires a context. Must appear as the first parameter in the function */
#define CTX_ARG Context& CTX

/** A context is an object that is passed around between many functions, similar to the "this" pointer.
 * It provides a common place to put data that is often considered static or global. It can also expose
 * extremely common utilities, including logging, assertions, and temporary allocations.
 */
struct Context {
	Context( size_t inTempCapacity )
		: threadID( std::this_thread::get_id() )
		, temp( inTempCapacity )
	{}

private:
	/** ID of the thread this context is being used on. A context should only be used by a single thread. */
	std::thread::id threadID;

public:
	/** Buffer used for dynamic allocation of small objects within the thread */
	HeapBuffer temp;
	/** Logger object used to print output */
	Logger log;

	inline std::thread::id GetThreadID() const { return threadID; }
};
